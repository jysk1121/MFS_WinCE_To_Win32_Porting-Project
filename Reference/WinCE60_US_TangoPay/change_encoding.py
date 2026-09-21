import os
import sys
import shutil
import chardet

FILE_TYPES = [".h", ".c", ".cpp", ".dat"]

EXCLUDED_ENCODINGS = ['ascii', ]
EXCLUDED_FILENAMES = [
    'KeyMgrText.dat',
    'KeyMgr.dat',
    'MultiTextKeyMgr.dat',
    'WinAtm.rc',
    'Device.dat',
]

KNOWN_LOCKED_FILES = {
    'ScreenDef.h': 'UTF-16',
    'BaseDef_CA.h': 'utf-8',
    'BaseDef_US.h': 'utf-8',
    'BaseDef_AU.h': 'utf-8',
    'BaseDef_MX.h': 'utf-8',
}

TEMPFILENAME = 'temp'


def _detect_file_encoding(file):
    """
    Detects the file encoding

    """
    return chardet.detect(file.read())


def _update_file_from_encoding(filename, src_encoding, dst_encoding):
    with open(TEMPFILENAME, 'w+', encoding=dst_encoding) as tempfile:
            with open(filename, 'r', encoding=src_encoding) as oldfile:
                tempfile.write(oldfile.read())

    shutil.move(TEMPFILENAME, filename)


def _update_locked_file(filepath):
    """
    Updates the encoding of the known locked file to that specified.

    """

    filename = os.path.basename(filepath)

    with open(filepath, 'rb') as file:
        encoding = _detect_file_encoding(file)

    src_encoding = encoding['encoding']
    dest_encoding = KNOWN_LOCKED_FILES[filename]

    if src_encoding != dest_encoding:
        print(f'{filename} -> {src_encoding} [locked to {dest_encoding}]')
        _update_file_from_encoding(filepath, src_encoding, dest_encoding)


def _update_file_to_encoding(filename, dst_encoding):
    """
    Detects the encoding of the source file, and changes it to the
    destination encoding

    """

    with open(filename, 'rb') as file:
        encoding = _detect_file_encoding(file)

    file_encoding = encoding["encoding"]
    if file_encoding != dst_encoding and file_encoding not in EXCLUDED_ENCODINGS:
        print(f'{filename} -> {file_encoding}')
        _update_file_from_encoding(filename, file_encoding, dst_encoding)


if __name__  == '__main__':

    # Arg 1 is new encoding
    new_enc = sys.argv[1]

    for subdir, dirs, files in os.walk('.'):
        for filename in files:

            if filename in EXCLUDED_FILENAMES:
                # Ignore excluded files
                continue

            filepath = subdir + os.sep + filename

            if filename in KNOWN_LOCKED_FILES.keys():
                _update_locked_file(filepath)
                continue

            for filetype in FILE_TYPES:
                if filepath.endswith(filetype):
                    _update_file_to_encoding(filepath, new_enc)
