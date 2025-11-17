# Web Content Build System

This project uses a build system to separate HTML, CSS, and JavaScript files for easier development, then embeds them into the Arduino `.ino` file when building.

## File Structure

```
nodemcu/
├── nodemcu.ino          # Main Arduino sketch (contains embedded HTML)
├── build_web.py         # Build script that embeds web files
└── web/                 # Separate web files for development
    ├── index.html       # HTML structure
    ├── style.css        # CSS styles
    └── script.js        # JavaScript functionality
```

## How It Works

1. **Development**: Edit the separate files in the `web/` directory:
   - `web/index.html` - HTML structure (uses `<link>` and `<script>` tags)
   - `web/style.css` - All CSS styles
   - `web/script.js` - All JavaScript code

2. **Build**: Run the build script to combine everything:
   ```bash
   python3 build_web.py
   ```

3. **Result**: The script:
   - Reads `web/index.html`, `web/style.css`, and `web/script.js`
   - Combines them into a single HTML file with inline CSS and JS
   - Replaces the placeholder section in `nodemcu.ino` with the embedded content
   - The embedded content is stored as a C string using `PROGMEM` for efficient memory usage

## Usage

### Before Building/Uploading

Always run the build script before compiling and uploading to your ESP8266:

```bash
cd nodemcu
python3 build_web.py
```

Then compile and upload `nodemcu.ino` as usual in the Arduino IDE.

### Editing Web Content

1. Edit files in the `web/` directory:
   - `web/index.html` - Modify HTML structure
   - `web/style.css` - Update styles
   - `web/script.js` - Change JavaScript behavior

2. Run the build script:
   ```bash
   python3 build_web.py
   ```

3. The `.ino` file is automatically updated with your changes.

### Important Notes

- **Don't edit the embedded HTML section in `nodemcu.ino` manually** - it will be overwritten by the build script
- The section between `// AUTO-GENERATED WEB CONTENT START` and `// AUTO-GENERATED WEB CONTENT END` is automatically generated
- The build script handles combining CSS and JS into the HTML automatically
- The HTML file uses `<link rel="stylesheet" href="/style.css">` and `<script src="/script.js">` tags, which are replaced with inline content during build

## Requirements

- Python 3 (for running the build script)
- Standard library only (no external dependencies)

## Troubleshooting

### Build script fails
- Make sure `web/index.html`, `web/style.css`, and `web/script.js` exist
- Check that Python 3 is installed: `python3 --version`

### Changes not appearing
- Make sure you ran `build_web.py` after editing web files
- Verify the build script completed successfully
- Recompile and upload the `.ino` file to your device

### Syntax errors in generated code
- Check that your HTML, CSS, and JS files are valid
- The build script preserves the exact content, so any syntax errors in source files will appear in the embedded version

