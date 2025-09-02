// swift-tools-version: 6.2
// The swift-tools-version declares the minimum version of Swift required to build this package.

import PackageDescription

let package = Package(
    name: "RsPack",
    platforms: [
        .macOS(.v15)
    ],
    products: [
        .library(
            name: "RsPack",
            targets: [
                "POLE",
                "Zlib",
                "LibJPEGTurbo",
                "LibPNG",
                "LibTIFF",
            ],
        ),
    ],
    targets: [
        .testTarget(
            name: "RsPackTests",         
            dependencies: [
            	"POLE",
                "Zlib",
                "LibJPEGTurbo",
                "LibPNG",
                "LibTIFF",
            ],
            resources: [
            	.copy("Resources/"),
            ],
            swiftSettings: [.interoperabilityMode(.Cxx)],
        ),
        .target(
            name: "POLE",
            dependencies: [
                "CPOLE",
            ],
            swiftSettings: [.interoperabilityMode(.Cxx)],
        ),
        .target(
            name: "Zlib",
            dependencies: [
                "CZlibNg",
            ],
            linkerSettings: [
                .unsafeFlags(["-L\(Context.packageDirectory)/Sources/CZlibNg/Lib"]),
            ],
        ),
        .target(
            name: "LibJPEGTurbo",
            dependencies: [
                "CLibJPEGTurbo",
            ],
            linkerSettings: [
                .unsafeFlags(["-L\(Context.packageDirectory)/Sources/CLibJPEGTurbo/Lib"]),
            ],
        ),
        .target(
            name: "LibPNG",
            dependencies: [
                "CLibPNG",
            ],
            linkerSettings: [
                .unsafeFlags(["-L\(Context.packageDirectory)/Sources/CLibPNG/Lib"]),
            ],
        ),
        .target(
            name: "LibTIFF",
            dependencies: [
                "CLibTIFF",
                "LibJPEGTurbo",
            ],
            linkerSettings: [
                .unsafeFlags(["-L\(Context.packageDirectory)/Sources/CLibTIFF/Lib"]),
            ],
        ),
        .target(
            name: "CPOLE",
            dependencies: [
            ],
            exclude: [
            ],
            sources: [
                "./Sources"
            ],
            cxxSettings: [
            ],
        ),
        .systemLibrary(
            name: "CZlibNg",
        ),
        .systemLibrary(
            name: "CLibJPEGTurbo",
        ),
        .systemLibrary(
            name: "CLibPNG",
        ),
        .systemLibrary(
            name: "CLibTIFF",
        ),
    ]
)
