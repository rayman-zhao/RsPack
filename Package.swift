// swift-tools-version: 6.2
// The swift-tools-version declares the minimum version of Swift required to build this package.

import PackageDescription

let package = Package(
    name: "RsPack",
    platforms: [
    	.macOS(.v15),
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
            	.target(name: "CZlibNg_x64-windows", condition: .when(platforms: [.windows])),
                .target(name: "CZlibNg", condition: .when(platforms: [.macOS])),
            ],
            linkerSettings: [
                .unsafeFlags(["-L\(Context.packageDirectory)/Sources/CZlibNg_x64-windows/Lib"], .when(platforms: [.windows])),
                .unsafeFlags(["-L\(Context.packageDirectory)/Sources/CZlibNg/Lib"], .when(platforms: [.macOS])),
            ],
        ),
        .target(
            name: "LibJPEGTurbo",
            dependencies: [
                .target(name: "CLibJPEGTurbo_x64-windows", condition: .when(platforms: [.windows])),
                .target(name: "CLibJPEGTurbo_arm64-macos", condition: .when(platforms: [.macOS])),
            ],
            linkerSettings: [
                .unsafeFlags(["-L\(Context.packageDirectory)/Sources/CLibJPEGTurbo_x64-windows/Lib"], .when(platforms: [.windows])),
                .unsafeFlags(["-L\(Context.packageDirectory)/Sources/CLibJPEGTurbo_arm64-macos/Lib"], .when(platforms: [.macOS])),
            ],
        ),
        .target(
            name: "LibPNG",
            dependencies: [
                .target(name: "CLibPNG_x64-windows", condition: .when(platforms: [.windows])),
                .target(name: "CLibPNG", condition: .when(platforms: [.macOS])),
            ],
            linkerSettings: [
                .unsafeFlags(["-L\(Context.packageDirectory)/Sources/CLibPNG_x64-windows/Lib"], .when(platforms: [.windows])),
                .unsafeFlags(["-L\(Context.packageDirectory)/Sources/CLibPNG/Lib"], .when(platforms: [.macOS])),
            ],
        ),
        .target(
            name: "LibTIFF",
            dependencies: [
                "LibJPEGTurbo",
                .target(name: "CLibTIFF_x64-windows", condition: .when(platforms: [.windows])),
                .target(name: "CLibTIFF", condition: .when(platforms: [.macOS])),
            ],
            linkerSettings: [
                .unsafeFlags(["-L\(Context.packageDirectory)/Sources/CLibTIFF_x64-windows/Lib"], .when(platforms: [.windows])),
                .unsafeFlags(["-L\(Context.packageDirectory)/Sources/CLibTIFF/Lib"], .when(platforms: [.macOS])),
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
            name: "CZlibNg_x64-windows",
        ),
        .systemLibrary(
            name: "CZlibNg",
        ),
        .systemLibrary(
            name: "CLibJPEGTurbo_x64-windows",
        ),
        .systemLibrary(
            name: "CLibJPEGTurbo_arm64-macos",
        ),
        .systemLibrary(
            name: "CLibPNG_x64-windows",
        ),
        .systemLibrary(
            name: "CLibPNG",
        ),
        .systemLibrary(
            name: "CLibTIFF_x64-windows",
        ),
        .systemLibrary(
            name: "CLibTIFF",
        ),
    ]
)
