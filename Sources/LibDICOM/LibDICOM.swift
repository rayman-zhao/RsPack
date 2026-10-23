import Foundation
import RsFoundation

@_exported import CLibDICOM

private let tagImageType = dcm_dict_tag_from_keyword("ImageType")
private let tagStudyInstanceUID = dcm_dict_tag_from_keyword("StudyInstanceUID")

/// A DICOM instance opened through libdicom.
///
/// libdicom's incomplete C struct types import as plain `OpaquePointer` on this
/// toolchain, so all Dcm* handles are untyped pointers.
///
/// The statically-linked libdicom carries no image codecs, and its
/// `dcm_is_encapsulated_transfer_syntax` treats every non-uncompressed transfer
/// syntax as encapsulated, so `readRawFrame(_:)` returns the raw, undecoded
/// fragment bytes (e.g. JPEG or HEVC codestreams) for them.
public final class DCMFile {
    private let handle: OpaquePointer

    /// Borrowed from the filehandle; valid for its lifetime, must not be destroyed.
    private lazy var metadata: OpaquePointer? = {
        var error: OpaquePointer?
        guard let meta = dcm_filehandle_get_metadata_subset(&error, handle) else {
            dcm_error_clear(&error)
            return nil
        }
        return meta
    }()

    public static func open(_ url: URL) -> DCMFile? {
        var error: OpaquePointer?
        // libdicom opens through the CRT narrow-char API, which decodes bytes in
        // the system ANSI/OEM codepage (GBK here), not UTF-8.
        guard
            let handle = dcm_filehandle_create_from_file(
                &error, url.filePath.oemCString)
        else {
            dcm_error_clear(&error)
            return nil
        }
        return DCMFile(handle: handle)
    }

    private init(handle: OpaquePointer) {
        self.handle = handle
    }

    deinit {
        dcm_filehandle_destroy(handle)
    }

    public func stringValue(_ tag: UInt32, index: UInt32 = 0) -> String? {
        guard let metadata else { return nil }
        var error: OpaquePointer?
        guard let element = dcm_dataset_get(&error, metadata, tag) else {
            dcm_error_clear(&error)
            return nil
        }

        var value: UnsafePointer<CChar>?
        guard dcm_element_get_value_string(&error, element, index, &value), let value
        else {
            dcm_error_clear(&error)
            return nil
        }
        return String(cString: value)
    }

    /// Image Type value 3, one of the VL whole-slide microscopy defined terms
    /// (`VOLUME`, `LABEL`, `OVERVIEW`, `THUMBNAIL`); nil when the file has fewer
    /// than three values.
    public var role: String? {
        stringValue(tagImageType, index: 2)
    }

    public var studyUID: String? {
        stringValue(tagStudyInstanceUID)
    }

    /// Undecoded frame bytes; frames are numbered from 1.
    public func readRawFrame(_ number: UInt32) -> [UInt8]? {
        var error: OpaquePointer?
        guard let frame = dcm_filehandle_read_frame(&error, handle, number) else {
            dcm_error_clear(&error)
            return nil
        }
        defer { dcm_frame_destroy(frame) }

        let length = Int(dcm_frame_get_length(frame))
        guard let value = dcm_frame_get_value(frame), length > 0 else { return nil }
        return [UInt8](
            UnsafeBufferPointer(
                start: UnsafeRawPointer(value).assumingMemoryBound(to: UInt8.self),
                count: length))
    }
}
