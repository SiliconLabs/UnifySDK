fn main() {
    // these exports are only used when an exe is compiled. e.g. integration
    // tests or final binaries!
    println!("cargo:rustc-link-lib=dylib=uic_attribute_store");
}
