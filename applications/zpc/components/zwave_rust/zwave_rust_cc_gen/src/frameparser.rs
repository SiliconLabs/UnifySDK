use std::collections::HashMap;
use std::convert::TryFrom;

/// FrameParser error type, this is returned by parsers if some error occurs
///
#[derive(Debug)]
pub struct FrameParserError {}

impl std::fmt::Display for FrameParserError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "frame parsing error")
    }
}

/// This structure is buffer containing the frame to be passed or generated
/// together with some parsing state
///
#[derive(Clone)]
pub struct FrameParser {
    /// FrameParser data both used when reading and writing
    data: Vec<u8>,
    /// Map of token positions
    ids: HashMap<u8, usize>,
    /// Number of bytes parsed when reding
    read_ptr: usize,
}

impl FrameParser {
    /// Append the frame d to this frame
    pub fn append(&mut self, d: FrameParser) {
        self.append_v(d.data);
    }

    /// Append vector data to this frame
    pub fn append_v(&mut self, d: Vec<u8>) {
        for e in d {
            self.data.push(e);
        }
    }

    /// Save the current write position as label
    pub fn save_write_label(&mut self, label: u8) {
        self.ids.insert(label, self.data.len());
    }

    /// Save the current read position as label
    pub fn save_read_label(&mut self, id: u8) {
        self.ids.insert(id, self.read_ptr);
    }

    /// update byte at position identified by label using a mask
    pub fn write_label(&mut self, label: u8, val: u8, mask: u8) {
        let offset = self.ids[&label];
        self.data[offset] = (self.data[offset] & !mask) | (val & mask);
    }

    /// Read byte at label with a mask
    pub fn read_label(&mut self, id: u8, mask: u8) -> u8 {
        let offset = self.ids[&id];
        self.data[offset] & mask
    }

    /// Return the raw frame data
    pub fn bytes(&self) -> &Vec<u8> {
        &self.data
    }

    /// Generate a new empty frame
    pub fn new() -> FrameParser {
        FrameParser {
            data: Vec::<u8>::new(),
            ids: HashMap::new(),
            read_ptr: 0,
        }
    }

    /// Read one byte of the frame and update the read pointer
    /// If we are at the end of the frame we will return 0 and the read pointer
    /// will not be updated
    pub fn read(&mut self) -> u8 {
        if self.read_ptr >= self.data.len() {
            0x00u8
        } else {
            let v = self.data[self.read_ptr];
            self.read_ptr = self.read_ptr + 1;
            v
        }
    }

    /// Copy the read pointer form one frame to another
    /// this is used when a copy of this frame has passed some frame data
    pub fn copy_state(&mut self, frame: &FrameParser) {
        self.read_ptr = frame.read_ptr;
    }
}

/// Structure to hold a 24 bit value
pub struct Bit24 {
    v: u32,
}

/////////////////////////////////////////////////////////////////////////////////////
/// From converter for standard data types to frames
/////////////////////////////////////////////////////////////////////////////////////
impl From<u8> for FrameParser {
    fn from(typ: u8) -> Self {
        let mut frame = FrameParser::new();
        frame.append_v(vec![typ]);
        frame
    }
}

impl From<u16> for FrameParser {
    fn from(typ: u16) -> Self {
        let mut frame = FrameParser::new();
        frame.append_v(vec![(typ >> 8) as u8, (typ & 0xff) as u8]);
        frame
    }
}

impl From<Bit24> for FrameParser {
    fn from(typ: Bit24) -> Self {
        let mut frame = FrameParser::new();
        frame.append_v(vec![
            ((typ.v >> 16) & 0xff) as u8,
            ((typ.v >> 8) & 0xff) as u8,
            ((typ.v >> 0) & 0xff) as u8,
        ]);
        frame
    }
}

impl From<u32> for FrameParser {
    fn from(typ: u32) -> Self {
        let mut frame = FrameParser::new();
        frame.append_v(vec![
            ((typ >> 24) & 0xff) as u8,
            ((typ >> 16) & 0xff) as u8,
            ((typ >> 8) & 0xff) as u8,
            ((typ >> 0) & 0xff) as u8,
        ]);
        frame
    }
}

impl<const N: usize> From<[u8; N]> for FrameParser {
    fn from(typ: [u8; N]) -> Self {
        let mut frame = FrameParser::new();
        frame.append_v(typ.into());
        frame
    }
}

impl From<&[u8]> for FrameParser {
    fn from(typ: &[u8]) -> Self {
        let mut frame = FrameParser::new();
        frame.append_v(typ.into());
        frame
    }
}

/////////////////////////////////////////////////////////////////////////////////////
/// From converter for a FrameParser to standard data types
/////////////////////////////////////////////////////////////////////////////////////
impl TryFrom<&mut FrameParser> for u8 {
    type Error = FrameParserError;
    fn try_from(frame: &mut FrameParser) -> Result<Self, Self::Error> {
        Ok(frame.read())
    }
}

impl TryFrom<&mut FrameParser> for u16 {
    type Error = FrameParserError;
    fn try_from(frame: &mut FrameParser) -> Result<Self, Self::Error> {
        let hi = frame.read();
        let lo = frame.read();
        Ok(((hi as u16) << 8) | (lo as u16))
    }
}

impl TryFrom<&mut FrameParser> for Bit24 {
    type Error = FrameParserError;
    fn try_from(frame: &mut FrameParser) -> Result<Self, Self::Error> {
        let b1 = frame.read();
        let b2 = frame.read();
        let b3 = frame.read();
        let v = ((b1 as u32) << 16) | ((b2 as u32) << 8) | ((b3 as u32) << 0);
        Ok(Bit24 { v })
    }
}

impl TryFrom<&mut FrameParser> for u32 {
    type Error = FrameParserError;
    fn try_from(frame: &mut FrameParser) -> Result<Self, Self::Error> {
        let b1 = frame.read();
        let b2 = frame.read();
        let b3 = frame.read();
        let b4 = frame.read();

        Ok(((b1 as u32) << 24) | ((b2 as u32) << 16) | ((b3 as u32) << 8) | ((b4 as u32) << 0))
    }
}

impl<const N: usize> TryFrom<&mut FrameParser> for [u8; N] {
    type Error = FrameParserError;
    fn try_from(frame: &mut FrameParser) -> Result<Self, Self::Error> {
        let mut v: [u8; N] = [0; N];
        for i in 0..N {
            v[i] = frame.read();
        }
        Ok(v)
    }
}
