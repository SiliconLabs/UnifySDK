///////////////////////////////////////////////////////////////////////////////
// # License
// <b>Copyright 2022  Silicon Laboratories Inc. www.silabs.com</b>
///////////////////////////////////////////////////////////////////////////////
// The licensor of this software is Silicon Laboratories Inc. Your use of this
// software is governed by the terms of Silicon Labs Master Software License
// Agreement (MSLA) available at
// www.silabs.com/about-us/legal/master-software-license-agreement. This
// software is distributed to you in Source Code format and is governed by the
// sections of the MSLA applicable to Source Code.
//
///////////////////////////////////////////////////////////////////////////////

use gag::BufferRedirect;
use std::io::Read;

pub struct Logging {
    log_lines: Vec<String>,
    stdout_buf: Option<BufferRedirect>,
    stderr_buf: Option<BufferRedirect>,
}

impl Logging {
    pub fn new(redirect: bool) -> std::io::Result<Self> {
        let mut stdout_buf = None;
        let mut stderr_buf = None;

        if redirect {
            stdout_buf = Some(BufferRedirect::stdout()?);
            stderr_buf = Some(BufferRedirect::stderr()?);
        }

        Ok(Logging {
            log_lines: Vec::new(),
            stdout_buf,
            stderr_buf,
        })
    }

    pub fn log(&mut self, log: String) {
        self.log_lines.push(log);
    }

    pub fn dump(&mut self) {
        self.stderr_buf = None;
        self.stdout_buf = None;

        println!("{}", self.log_lines.join("\n"));
    }

    pub fn error(&mut self) {
        let mut std_err = String::new();
        let mut std_out = String::new();

        if let Some(mut buffer) = self.stderr_buf.take() {
            buffer.read_to_string(&mut std_err).unwrap();
        }

        if let Some(mut buffer) = self.stdout_buf.take() {
            buffer.read_to_string(&mut std_out).unwrap();
        }

        println!("STDOUT: {}", std_out);
        println!("STDERR: {}", std_err);
    }
}
