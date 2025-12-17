#!/usr/bin/env python3
"""
Generate ISO metadata JSON for Bazzite downloads.
This script fetches available ISOs from download.bazzite.gg and creates
a JSON structure similar to Fedora's getfedora.org API.
"""

import argparse
import json
import re
import sys
from pathlib import Path
from typing import Dict, List, Optional
from urllib.parse import urljoin

import requests
from bs4 import BeautifulSoup


BASE_URL = "https://download.bazzite.gg/"


def parse_iso_filename(filename: str) -> Optional[Dict[str, str]]:
    """
    Parse Bazzite ISO filename to extract metadata.
    
    Format: bazzite[-variant][-branch]-amd64.iso
    Examples:
        - bazzite-stable-amd64.iso
        - bazzite-nvidia-stable-amd64.iso
        - bazzite-deck-gnome-testing-amd64.iso
    """
    # Remove .iso extension
    name = filename.replace('.iso', '')
    
    # Pattern: bazzite[-variant][-branch]-[arch]
    # Branch is typically 'stable' or 'testing'
    # Arch is typically 'amd64' or 'arm64'
    pattern = r'^bazzite(?:-(.+?))?-(stable|testing)-([a-z0-9]+)$'
    match = re.match(pattern, name)
    
    if not match:
        return None
    
    variant_part = match.group(1) if match.group(1) else None
    branch = match.group(2)
    arch = match.group(3)
    
    # Convert arch names
    arch_map = {
        'amd64': 'x86_64',
        'arm64': 'aarch64',
    }
    arch = arch_map.get(arch, arch)
    
    # Parse variant and subvariant
    variant = "Bazzite"
    subvariant = "KDE"  # Default
    
    if variant_part:
        parts = variant_part.split('-')
        
        # Map common patterns
        if 'gnome' in parts:
            subvariant = "GNOME"
            parts.remove('gnome')
        
        if 'nvidia' in parts:
            if 'open' in parts:
                subvariant = f"{subvariant}_NVIDIA_Open"
            else:
                subvariant = f"{subvariant}_NVIDIA"
        
        # Special variants
        if 'deck' in parts:
            variant = "Bazzite_Deck"
        elif 'ally' in parts:
            variant = "Bazzite_Ally"
        elif 'asus' in parts:
            variant = "Bazzite_ASUS"
        elif 'surface' in parts:
            variant = "Bazzite_Surface"
    
    return {
        'variant': variant,
        'subvariant': subvariant,
        'branch': branch,
        'arch': arch,
        'filename': filename
    }


def fetch_checksum(base_url: str, filename: str) -> Optional[str]:
    """Fetch SHA256 checksum for an ISO file."""
    checksum_url = urljoin(base_url, f"{filename}-CHECKSUM")
    
    try:
        response = requests.get(checksum_url, timeout=10)
        response.raise_for_status()
        
        # Parse checksum file (format: SHA256 (filename) = hash)
        content = response.text.strip()
        
        # Try various checksum formats
        # Format 1: SHA256 (filename) = hash
        match = re.search(r'SHA256\s*\([^)]+\)\s*=\s*([a-f0-9]{64})', content, re.IGNORECASE)
        if match:
            return match.group(1).lower()
        
        # Format 2: hash filename
        match = re.search(r'([a-f0-9]{64})', content)
        if match:
            return match.group(1).lower()
        
        return None
    except Exception as e:
        print(f"Warning: Could not fetch checksum for {filename}: {e}", file=sys.stderr)
        return None


def get_file_size(url: str) -> Optional[int]:
    """Get file size from Content-Length header."""
    try:
        response = requests.head(url, timeout=10, allow_redirects=True)
        response.raise_for_status()
        
        content_length = response.headers.get('Content-Length')
        if content_length:
            return int(content_length)
        
        return None
    except Exception as e:
        print(f"Warning: Could not get file size for {url}: {e}", file=sys.stderr)
        return None


def scrape_available_isos(base_url: str) -> List[str]:
    """
    Scrape available ISO files from download.bazzite.gg.
    
    This assumes the bucket has directory listing enabled or uses a simple HTML index.
    """
    try:
        response = requests.get(base_url, timeout=10)
        response.raise_for_status()
        
        soup = BeautifulSoup(response.text, 'html.parser')
        
        # Find all links ending in .iso
        iso_files = []
        for link in soup.find_all('a', href=True):
            href = link['href']
            if href.endswith('.iso'):
                # Extract just the filename
                filename = href.split('/')[-1]
                iso_files.append(filename)
        
        return iso_files
    
    except Exception as e:
        print(f"Error: Could not fetch ISO list from {base_url}: {e}", file=sys.stderr)
        return []


def list_isos_from_file(file_path: Path) -> List[str]:
    """Read ISO filenames from a text file (one per line)."""
    try:
        with open(file_path, 'r') as f:
            return [line.strip() for line in f if line.strip() and line.strip().endswith('.iso')]
    except Exception as e:
        print(f"Error reading file {file_path}: {e}", file=sys.stderr)
        return []


def generate_iso_metadata(base_url: str, iso_files: List[str], version: str = "43") -> List[Dict]:
    """Generate metadata for all ISO files."""
    metadata = []
    
    for filename in iso_files:
        print(f"Processing {filename}...", file=sys.stderr)
        
        # Parse filename
        parsed = parse_iso_filename(filename)
        if not parsed:
            print(f"Warning: Could not parse filename: {filename}", file=sys.stderr)
            continue
        
        # Build full URL
        iso_url = urljoin(base_url, filename)
        
        # Fetch checksum
        checksum = fetch_checksum(base_url, filename)
        
        # Get file size
        file_size = get_file_size(iso_url)
        
        # Build metadata entry
        entry = {
            "version": version,
            "arch": parsed['arch'],
            "link": iso_url,
            "variant": parsed['variant'],
            "subvariant": parsed['subvariant'],
            "branch": parsed['branch'],
            "sha256": checksum,
            "size": str(file_size) if file_size else None
        }
        
        metadata.append(entry)
    
    return metadata


def main():
    parser = argparse.ArgumentParser(
        description="Generate ISO metadata JSON for Bazzite downloads"
    )
    parser.add_argument(
        '--base-url',
        default=BASE_URL,
        help=f"Base URL for downloads (default: {BASE_URL})"
    )
    parser.add_argument(
        '--version',
        default='43',
        help="Fedora version number (default: 43)"
    )
    parser.add_argument(
        '--input-file',
        type=Path,
        help="Read ISO filenames from a text file instead of scraping"
    )
    parser.add_argument(
        '--output',
        type=Path,
        help="Output JSON file (default: stdout)"
    )
    parser.add_argument(
        '--pretty',
        action='store_true',
        help="Pretty-print JSON output"
    )
    
    args = parser.parse_args()
    
    # Get list of ISOs
    if args.input_file:
        print(f"Reading ISO list from {args.input_file}...", file=sys.stderr)
        iso_files = list_isos_from_file(args.input_file)
    else:
        print(f"Scraping ISO list from {args.base_url}...", file=sys.stderr)
        iso_files = scrape_available_isos(args.base_url)
    
    if not iso_files:
        print("Error: No ISO files found", file=sys.stderr)
        return 1
    
    print(f"Found {len(iso_files)} ISO files", file=sys.stderr)
    
    # Generate metadata
    metadata = generate_iso_metadata(args.base_url, iso_files, args.version)
    
    # Output JSON
    json_kwargs = {'indent': 2} if args.pretty else {}
    json_output = json.dumps(metadata, **json_kwargs)
    
    if args.output:
        args.output.write_text(json_output)
        print(f"Metadata written to {args.output}", file=sys.stderr)
    else:
        print(json_output)
    
    return 0


if __name__ == '__main__':
    sys.exit(main())
