#!/usr/bin/env python3
"""
Generate NSIS language file from PO translations.
"""

import glob
import os
import re
import sys
import polib
from collections import defaultdict

# Mapping from gettext language codes to NSIS language codes
LANGUAGE_MAP = {
    'af': 'AFRIKAANS',
    'sq': 'ALBANIAN',
    'ar': 'ARABIC',
    'be': 'BELARUSIAN',
    'bs': 'BOSNIAN',
    'br': 'BRETON',
    'bg': 'BULGARIAN',
    'ca': 'CATALAN',
    'hr': 'CROATIAN',
    'cs': 'CZECH',
    'da': 'DANISH',
    'nl': 'DUTCH',
    'eo': 'ESPERANTO',
    'et': 'ESTONIAN',
    'fa': 'FARSI',
    'fi': 'FINNISH',
    'fr': 'FRENCH',
    'gl': 'GALICIAN',
    'de': 'GERMAN',
    'el': 'GREEK',
    'he': 'HEBREW',
    'hu': 'HUNGARIAN',
    'is': 'ICELANDIC',
    'id': 'INDONESIAN',
    'ga': 'IRISH',
    'it': 'ITALIAN',
    'ja': 'JAPANESE',
    'ko': 'KOREAN',
    'ku': 'KURDISH',
    'lv': 'LATVIAN',
    'lt': 'LITHUANIAN',
    'lb': 'LUXEMBOURGISH',
    'mk': 'MACEDONIAN',
    'ms': 'MALAY',
    'mn': 'MONGOLIAN',
    'nb': 'NORWEGIAN',
    'nn': 'NORWEGIANNYNORSK',
    'pl': 'POLISH',
    'pt': 'PORTUGUESE',
    'pt-BR': 'PORTUGUESEBR',
    'pt_BR': 'PORTUGUESEBR',
    'ro': 'ROMANIAN',
    'ru': 'RUSSIAN',
    'sr': 'SERBIAN',
    'sr-Latn': 'SERBIANLATIN',
    'zh-CN': 'SIMPCHINESE',
    'zh_CN': 'SIMPCHINESE',
    'sk': 'SLOVAK',
    'sl': 'SLOVENIAN',
    'es': 'SPANISH',
    'es-ES': 'SPANISHINTERNATIONAL',
    'sv': 'SWEDISH',
    'th': 'THAI',
    'zh-TW': 'TRADCHINESE',
    'zh_TW': 'TRADCHINESE',
    'tr': 'TURKISH',
    'uk': 'UKRAINIAN',
    'uz': 'UZBEK',
}

# All NSIS languages (for complete coverage)
ALL_NSIS_LANGUAGES = [
    'AFRIKAANS', 'ALBANIAN', 'ARABIC', 'BELARUSIAN', 'BOSNIAN', 'BRETON',
    'BULGARIAN', 'CATALAN', 'CROATIAN', 'CZECH', 'DANISH', 'DUTCH',
    'ESPERANTO', 'ESTONIAN', 'FARSI', 'FINNISH', 'FRENCH', 'GALICIAN',
    'GERMAN', 'GREEK', 'HEBREW', 'HUNGARIAN', 'ICELANDIC', 'INDONESIAN',
    'IRISH', 'ITALIAN', 'JAPANESE', 'KOREAN', 'KURDISH', 'LATVIAN',
    'LITHUANIAN', 'LUXEMBOURGISH', 'MACEDONIAN', 'MALAY', 'MONGOLIAN',
    'NORWEGIAN', 'NORWEGIANNYNORSK', 'POLISH', 'PORTUGUESE', 'PORTUGUESEBR',
    'ROMANIAN', 'RUSSIAN', 'SERBIAN', 'SERBIANLATIN', 'SIMPCHINESE',
    'SLOVAK', 'SLOVENIAN', 'SPANISH', 'SPANISHINTERNATIONAL', 'SWEDISH',
    'THAI', 'TRADCHINESE', 'TURKISH', 'UKRAINIAN', 'UZBEK',
]

def read_po_files(po_dir, english_strings):
    """Read all PO files and extract NSIS translations."""

    # Initialize with English strings
    translations = defaultdict(lambda: defaultdict(str))
    for string_id, english_text in english_strings.items():
        translations[string_id]['ENGLISH'] = english_text

    # Read all PO files
    po_files = glob.glob(os.path.join(po_dir, 'mediawriter_*.po'))
    for po_file in po_files:
        # Extract language code from filename
        basename = os.path.basename(po_file)
        lang_code = basename.replace('mediawriter_', '').replace('.po', '')

        # Map to NSIS language code
        nsis_lang = LANGUAGE_MAP.get(lang_code)
        if not nsis_lang:
            print(f'Warning: No NSIS mapping for language {lang_code}, skipping {po_file}', file=sys.stderr)
            continue

        # Parse PO file
        try:
            po = polib.pofile(po_file)
            for entry in po:
                # Check if this is an NSIS string
                if entry.msgctxt and entry.msgctxt.startswith('NSIS:'):
                    string_id = entry.msgctxt.replace('NSIS:', '')
                    if entry.msgstr:
                        translations[string_id][nsis_lang] = entry.msgstr
        except Exception as e:
            print(f'Error reading {po_file}: {e}', file=sys.stderr)

    return translations

def write_nsis_file(translations, output_file):
    """Write translations to NSIS language file."""
    # Get list of all string IDs
    string_ids = sorted(translations.keys())

    # Create list with ENGLISH first, then all others
    all_languages = ['ENGLISH'] + ALL_NSIS_LANGUAGES

    with open(output_file, 'w', encoding='utf-8') as f:
        for i, string_id in enumerate(string_ids):
            if i > 0:
                f.write('\n')

            # Get English string as fallback
            english_text = translations[string_id].get('ENGLISH', '')

            # Write LangString for each language (ENGLISH first, then others)
            for nsis_lang in all_languages:
                # Get translation or fall back to English
                text = translations[string_id].get(nsis_lang, english_text)

                # Escape for NSIS
                text = text.replace('\\n', '$\\n')
                text = text.replace('\\t', '$\\t')
                # Escape unescaped double quotes, but leave existing NSIS escapes ($\\") intact
                text = re.sub(r'(?<!\$\\)"', r'$\\\"', text)

                # Write LangString with fixed 2 spaces between language and quote
                f.write(f'LangString {string_id} ${{LANG_{nsis_lang}}}  "{text}"\n')

def extract_english_strings(nsis_file):
    """Extract English strings from existing NSIS file."""
    strings = {}

    with open(nsis_file, 'r', encoding='utf-8') as f:
        for line in f:
            match = re.match(r'\s*LangString\s+(\w+)\s+\$\{LANG_ENGLISH\}\s+"(.+)"', line)
            if match:
                string_id = match.group(1)
                string_value = match.group(2)
                # Unescape NSIS escaped characters
                string_value = string_value.replace('$\\n', '\\n')
                string_value = string_value.replace('$\\t', '\\t')
                string_value = string_value.replace('$\\"', '"')
                strings[string_id] = string_value

    return strings

if __name__ == '__main__':
    if len(sys.argv) != 4:
        print(f'Usage: {sys.argv[0]} <original-nsis-file> <po-directory> <output-nsis-file>')
        sys.exit(1)

    original_nsis = sys.argv[1]
    po_dir = sys.argv[2]
    output_file = sys.argv[3]

    # Extract English strings from original file
    english_strings = extract_english_strings(original_nsis)
    print(f'Extracted {len(english_strings)} English strings from {original_nsis}')

    # Read translations from PO files
    translations = read_po_files(po_dir, english_strings)

    # Write output file
    write_nsis_file(translations, output_file)

    print(f'Generated {output_file} with translations')
