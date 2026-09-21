import os
import io
import re


def korean_encoding_error_on_line(filename):
    try:
        line_number = 1
        with io.open(path, 'r', encoding='utf-8') as file:
            for line in file.readlines():
                new = line.encode('euc_kr')
                line_number += 1
        return 0
    except UnicodeDecodeError as ex:
        return 0
    except UnicodeEncodeError as ex:
        return line_number

def reencode_file(filename):
    """
    Re-encodes the file with UTF-8 for korean character support\
    
    """
    temp_file = filename + '.bak'
    try:
        with io.open(filename, 'rb') as input:
            with io.open(temp_file, 'w', encoding='utf-8', newline='') as output:
                for line in input.readlines():
                    new = line.decode('euc_kr')
                    output.write(new)
        os.replace(temp_file, filename)
    except UnicodeDecodeError as ex:
        error_line_number = korean_encoding_error_on_line(filename)
        if error_line_number != 0:
            print(filename, " => Error encoding file", ex, ": line number:", error_line_number)

    if os.path.exists(temp_file):
        os.remove(temp_file)


if __name__ == '__main__':
    source_files = re.compile(r'.*\.(cpp|h|hpp)$')

    # Walk files and re-encode
    for dir, dirs, files in os.walk('./'):
        for filename in files:
            if source_files.match(filename):
                path = os.path.join(dir, filename)
                reencode_file(path)
