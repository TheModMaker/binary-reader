# Binary File Reader

_This is not an officially supported Google product._

This project defines a custom language to describe a binary file format.  Using
this, this project can extract, modify, or dump these fields from a binary file.
For eample, you can create a description of an MP4 file and use this tool to
extract a certain piece of that file, or just dump all the fields as JSON.

This project exposes both a C++ API and a command-line utility to perform
common tasks.


## Language format

The file specification is made up of types which define sections of the file.
Types are defined using fields, which are binary elements such as numbers or
strings.  Within a type, you can use `if` statements to create conditional
fields, `while` loops for repeated fields, and more.


## Command-line examples

```sh
# Dump all the fields from the file.
binary_reader -f format.idl -i file.bin --dump

# Extract the given field
binary_reader -f format.idl -i file.bin -x "people[0].first_name"

# Extract all the given fields
binary_reader -f format.idl -i file.bin -x "people[*].friend_ids"
```


## Example format definition

__Not formalized yet, format subject to change.  Not all features implemented.__

```
option encoding = 'utf8';

type Person {
  uint16 id;
  byte[9] ssn;

  // 'cstring' type is a null-terminated array of bytes.
  cstring first_name;
  cstring last_name;

  uint32 friend_count;
  uint16[friend_count] friend_ids;
}

// The last type is (by default) the whole file.
type File {
  uint32 version;

  if (version > 0) {
    uint64 created_timestamp;
  }

  // Use the remaining file for an array of Person types.
  Person[*] people;
}
```

A more complex example, for MP4 files:

```
type Box<BoxType> {
  uint32 size;
  // If set, expects this field to have this value.  Can also be unset ('*'),
  // which means this field can be anything.
  uint32 type = BoxType;

  // Special variable for this types size.  If not set, will use the fields to
  // detect the size.  This allows a field of type Box<*> to detect how big
  // it is without knowing the kind of box.
  set $size = size;
}

type FullBox<BoxType> : Box<BoxType> {
  byte version;
  integer(unsigned, 24) flags;  // Uncommon integer size, in bits.
}

type FileType : Box<'ftyp'> {
  uint32 major_brand;
}

type SampleToChunkBox : FullBox<'stsc'> {
  uint32 entry_count;
  for (i = 1; i <= entry_count; i++) {
    uint32 first_chunk;
    uint32 samples_per_chunk;
    uint32 sample_description_index;
  }
}

type Mp4File {
  // Box<*> specifies to use the Box type with the special "any" value as the
  // template parameter.
  Box<*>[*] boxes;
}
```
