@_exported import CLibDICOM
import Foundation
import RsFoundation

/// Tag numbers of the elements the LibDICOM helpers read, resolved through the
/// library's own dictionary.
public enum DCMTag {
    public static let imageType = dcm_dict_tag_from_keyword("ImageType")
    public static let sopInstanceUID = dcm_dict_tag_from_keyword("SOPInstanceUID")
    public static let studyInstanceUID = dcm_dict_tag_from_keyword("StudyInstanceUID")
    public static let rows = dcm_dict_tag_from_keyword("Rows")
    public static let columns = dcm_dict_tag_from_keyword("Columns")
    public static let totalPixelMatrixColumns = dcm_dict_tag_from_keyword(
        "TotalPixelMatrixColumns")
    public static let totalPixelMatrixRows = dcm_dict_tag_from_keyword("TotalPixelMatrixRows")
    public static let pixelSpacing = dcm_dict_tag_from_keyword("PixelSpacing")
    public static let sharedFunctionalGroupsSequence = dcm_dict_tag_from_keyword(
        "SharedFunctionalGroupsSequence")
    public static let pixelMeasuresSequence = dcm_dict_tag_from_keyword("PixelMeasuresSequence")
    public static let opticalPathSequence = dcm_dict_tag_from_keyword("OpticalPathSequence")
    public static let objectiveLensPower = dcm_dict_tag_from_keyword("ObjectiveLensPower")
}

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

    public func stringValue(_ tag: UInt32, index: UInt32 = 0, in dataset: OpaquePointer? = nil)
        -> String?
    {
        guard let element = element(tag, dataset) else { return nil }
        return string(element, index: index)
    }

    public func intValue(_ tag: UInt32, index: UInt32 = 0, in dataset: OpaquePointer? = nil) -> Int?
    {
        guard let element = element(tag, dataset) else { return nil }

        var error: OpaquePointer?
        var value = Int64(0)
        if dcm_element_get_value_integer(&error, element, index, &value) {
            return Int(value)
        }
        dcm_error_clear(&error)
        // File-parsed IS elements keep their string form, which the numeric
        // getter rejects; parse it ourselves.
        if let s = string(element, index: index)?.trimmingCharacters(in: .whitespaces) {
            return Int(s)
        }
        return nil
    }

    public func decimalValue(_ tag: UInt32, index: UInt32 = 0, in dataset: OpaquePointer? = nil)
        -> Double?
    {
        guard let element = element(tag, dataset) else { return nil }

        var error: OpaquePointer?
        var value = Double(0)
        if dcm_element_get_value_decimal(&error, element, index, &value) {
            return value
        }
        dcm_error_clear(&error)
        // File-parsed DS elements keep their string form, which the numeric
        // getter rejects; parse it ourselves.
        if let s = string(element, index: index)?.trimmingCharacters(in: .whitespaces) {
            return Double(s)
        }
        return nil
    }

    /// The dataset of a sequence item, borrowed from the receiver's metadata and
    /// valid while the file is open; nil when the sequence or item is absent.
    public func itemDataset(
        _ sequenceTag: UInt32, item: UInt32 = 0, in dataset: OpaquePointer? = nil
    ) -> OpaquePointer? {
        guard let element = element(sequenceTag, dataset) else { return nil }

        var error: OpaquePointer?
        var sequence: OpaquePointer?
        guard dcm_element_get_value_sequence(&error, element, &sequence), let sequence,
            let itemDataset = dcm_sequence_get(&error, sequence, item)
        else {
            dcm_error_clear(&error)
            return nil
        }
        return itemDataset
    }

    private func element(_ tag: UInt32, _ dataset: OpaquePointer?) -> OpaquePointer? {
        guard let base = dataset ?? metadata else { return nil }
        var error: OpaquePointer?
        guard let element = dcm_dataset_get(&error, base, tag) else {
            dcm_error_clear(&error)
            return nil
        }
        return element
    }

    private func string(_ element: OpaquePointer, index: UInt32) -> String? {
        var error: OpaquePointer?
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

    /// One-based frame number of the tile at a (column, row) position, taking
    /// frame-positioning metadata into account; nil when the tile is sparse or
    /// the lookup fails.
    public func frameNumber(column: UInt32, row: UInt32) -> UInt32? {
        var error: OpaquePointer?
        var number: UInt32 = 0
        guard
            dcm_filehandle_find_frame_number(&error, handle, column, row, &number),
            number > 0
        else {
            dcm_error_clear(&error)
            return nil
        }
        return number
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
