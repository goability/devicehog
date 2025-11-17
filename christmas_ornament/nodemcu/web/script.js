// Restore color picker value from URL parameters on page load
function restoreColorPicker() {
  const urlParams = new URLSearchParams(window.location.search);
  const r = urlParams.get('r');
  const g = urlParams.get('g');
  const b = urlParams.get('b');
  if (r !== null && g !== null && b !== null) {
    const rHex = parseInt(r).toString(16).padStart(2, '0');
    const gHex = parseInt(g).toString(16).padStart(2, '0');
    const bHex = parseInt(b).toString(16).padStart(2, '0');
    document.getElementById('colorPicker').value = '#' + rHex + gHex + bHex;
  }
}

function setRGBColor() {
  const color = document.getElementById('colorPicker').value;
  const r = parseInt(color.substring(1, 3), 16);
  const g = parseInt(color.substring(3, 5), 16);
  const b = parseInt(color.substring(5, 7), 16);
  window.location.href = '/rgb?r=' + r + '&g=' + g + '&b=' + b;
}

// Call on page load to restore color
window.onload = restoreColorPicker;

