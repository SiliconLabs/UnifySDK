fn main() -> Result<(), Box<dyn std::error::Error>> {
    // workaround: rpaths when declared in dependant crates are not transitive ?!
    // therefore redeclare them here
    println!(
        "cargo:rustc-link-arg=-Wl,-rpath,{}/components",
        std::env::var("UNIFY_BINARY_DIR")?
    );
    Ok(())
}
