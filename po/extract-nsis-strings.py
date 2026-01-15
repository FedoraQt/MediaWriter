#!/usr/bin/env python3
"""
Extract translatable strings from NSIS language files and output in POT format.
"""

import re
import sys
from datetime import datetime

def extract_nsis_strings(nsis_file):
    """Extract English strings from NSIS language file."""
    strings = {}

    with open(nsis_file, 'r', encoding='utf-8') as f:
        for line in f:
            # Match LangString lines with LANG_ENGLISH
            match = re.match(r'\s*LangString\s+(\w+)\s+\$\{LANG_ENGLISH\}\s+"(.+)"', line)
            if match:
                string_id = match.group(1)
                string_value = match.group(2)
                # Unescape NSIS escaped characters
                string_value = string_value.replace('$\\n', '\\n')
                string_value = string_value.replace('$\\t', '\\t')
                # Keep ${APPNAME} as-is for now, it will be handled by gettext
                strings[string_id] = string_value

    return strings

def write_pot_file(strings, output_file):
    """Write strings to POT file format."""
    with open(output_file, 'w', encoding='utf-8') as f:
        # Write POT header
        f.write('# NSIS Installer strings for MediaWriter\n')
        f.write('# Copyright (C) YEAR THE PACKAGE\'S COPYRIGHT HOLDER\n')
        f.write('# This file is distributed under the same license as the PACKAGE package.\n')
        f.write('# FIRST AUTHOR <EMAIL@ADDRESS>, YEAR.\n')
        f.write('#\n')
        f.write('#, fuzzy\n')
        f.write('msgid ""\n')
        f.write('msgstr ""\n')
        f.write('"Project-Id-Version: PACKAGE VERSION\\n"\n')
        f.write('"Report-Msgid-Bugs-To: \\n"\n')
        f.write(f'"POT-Creation-Date: {datetime.now().strftime("%Y-%m-%d %H:%M%z")}\\n"\n')
        f.write('"PO-Revision-Date: YEAR-MO-DA HO:MI+ZONE\\n"\n')
        f.write('"Last-Translator: FULL NAME <EMAIL@ADDRESS>\\n"\n')
        f.write('"Language-Team: LANGUAGE <LL@li.org>\\n"\n')
        f.write('"Language: \\n"\n')
        f.write('"MIME-Version: 1.0\\n"\n')
        f.write('"Content-Type: text/plain; charset=UTF-8\\n"\n')
        f.write('"Content-Transfer-Encoding: 8bit\\n"\n')
        f.write('\n')

        # Write each string with context
        for string_id, string_value in sorted(strings.items()):
            f.write(f'#: NSIS:{string_id}\n')
            f.write(f'msgctxt "NSIS:{string_id}"\n')
             # Escape quotes in the string, but avoid double-escaping already escaped quotes
            escaped_value = re.sub(r'(?<!\\\\)"', r'\\"', string_value)
            f.write(f'msgid "{escaped_value}"\n')
            f.write('msgstr ""\n')
            f.write('\n')

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print(f'Usage: {sys.argv[0]} <nsis-language-file> <output-pot-file>')
        sys.exit(1)

    nsis_file = sys.argv[1]
    output_file = sys.argv[2]

    strings = extract_nsis_strings(nsis_file)
    write_pot_file(strings, output_file)

    print(f'Extracted {len(strings)} strings from {nsis_file} to {output_file}')
