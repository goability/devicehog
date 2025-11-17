#!/usr/bin/env python3
"""
Build script to embed HTML, CSS, and JS files into the Arduino .ino file.

This script reads separate HTML, CSS, and JS files and combines them
into a single embedded string in the .ino file, replacing the placeholder.

Usage:
    python3 build_web.py

The script will:
1. Read web/index.html, web/style.css, and web/script.js
2. Combine them into a single HTML string with inline CSS and JS
3. Replace the placeholder in nodemcu.ino with the embedded content
"""

import os
import re
import sys

# File paths
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
WEB_DIR = os.path.join(SCRIPT_DIR, 'web')
INO_FILE = os.path.join(SCRIPT_DIR, 'nodemcu.ino')
HTML_FILE = os.path.join(WEB_DIR, 'index.html')
CSS_FILE = os.path.join(WEB_DIR, 'style.css')
JS_FILE = os.path.join(WEB_DIR, 'script.js')

# Placeholder markers in .ino file
START_MARKER = '// AUTO-GENERATED WEB CONTENT START - DO NOT EDIT MANUALLY'
END_MARKER = '// AUTO-GENERATED WEB CONTENT END'

def read_file_safe(filepath):
    """Read a file, return empty string if it doesn't exist."""
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            return f.read()
    except FileNotFoundError:
        print(f"Warning: {filepath} not found, using empty content")
        return ""

def embed_css_in_html(html_content, css_content):
    """Replace <link rel="stylesheet"> with inline <style> tag."""
    # Remove the link tag
    html_content = re.sub(
        r'<link\s+rel=["\']stylesheet["\']\s+href=["\']/style\.css["\']\s*>',
        '',
        html_content,
        flags=re.IGNORECASE
    )
    # Insert style tag in head
    style_tag = f'<style>\n{css_content}\n  </style>'
    html_content = re.sub(
        r'(</head>)',
        f'  {style_tag}\n\\1',
        html_content,
        flags=re.IGNORECASE
    )
    return html_content

def embed_js_in_html(html_content, js_content):
    """Replace <script src=""> with inline <script> tag."""
    # Remove the script tag
    html_content = re.sub(
        r'<script\s+src=["\']/script\.js["\']\s*></script>',
        '',
        html_content,
        flags=re.IGNORECASE
    )
    # Insert script tag before </body>
    script_tag = f'  <script>\n    {js_content.replace(chr(10), chr(10) + "    ")}\n  </script>'
    html_content = re.sub(
        r'(</body>)',
        f'{script_tag}\n\\1',
        html_content,
        flags=re.IGNORECASE
    )
    return html_content

def build_embedded_html():
    """Read HTML, CSS, JS files and combine them into a single HTML string."""
    html = read_file_safe(HTML_FILE)
    css = read_file_safe(CSS_FILE)
    js = read_file_safe(JS_FILE)
    
    # Embed CSS and JS into HTML
    html = embed_css_in_html(html, css)
    html = embed_js_in_html(html, js)
    
    return html

def update_ino_file(embedded_html):
    """Update the .ino file with the embedded HTML content."""
    # Read the .ino file
    with open(INO_FILE, 'r', encoding='utf-8') as f:
        ino_content = f.read()
    
    # Find the section between markers
    pattern = re.compile(
        re.escape(START_MARKER) + r'.*?' + re.escape(END_MARKER),
        re.DOTALL
    )
    
    # Format the embedded HTML for the C string
    # Escape any backslashes and quotes that might cause issues
    # The R"rawliteral" syntax handles most cases, but we need to be careful
    formatted_html = embedded_html
    
    # Create the replacement content
    replacement = f"""{START_MARKER}
const char index_html[] PROGMEM = R"rawliteral(
{formatted_html})rawliteral";
{END_MARKER}"""
    
    if pattern.search(ino_content):
        # Replace existing content
        ino_content = pattern.sub(replacement, ino_content)
    else:
        # Insert before the handleRoot function
        # Find a good insertion point (before handleRoot or isAuthenticated)
        insert_pattern = re.compile(
            r'(// Check if client is authenticated)',
            re.MULTILINE
        )
        if insert_pattern.search(ino_content):
            ino_content = insert_pattern.sub(
                replacement + '\n\n\\1',
                ino_content
            )
        else:
            # Fallback: insert after the www_password line
            insert_pattern = re.compile(
                r'(const char\* www_password = "[^"]*";)',
                re.MULTILINE
            )
            if insert_pattern.search(ino_content):
                ino_content = insert_pattern.sub(
                    '\\1\n\n' + replacement,
                    ino_content
                )
            else:
                print("Error: Could not find insertion point in .ino file")
                sys.exit(1)
    
    # Write the updated .ino file
    with open(INO_FILE, 'w', encoding='utf-8') as f:
        f.write(ino_content)
    
    print(f"✅ Successfully updated {INO_FILE}")

def main():
    """Main build function."""
    print("🔨 Building web content for Arduino...")
    
    # Check if web files exist
    if not os.path.exists(HTML_FILE):
        print(f"❌ Error: {HTML_FILE} not found")
        sys.exit(1)
    
    # Build embedded HTML
    embedded_html = build_embedded_html()
    
    if not embedded_html.strip():
        print("❌ Error: No content to embed")
        sys.exit(1)
    
    # Update .ino file
    update_ino_file(embedded_html)
    
    print("✅ Build complete!")

if __name__ == '__main__':
    main()

