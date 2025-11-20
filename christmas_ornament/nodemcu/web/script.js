// RGB color button handlers - buttons use direct links, no JS needed

// LED1 fade control functions
function updateFadeSpeed(speed) {
  document.getElementById('speedValue').textContent = speed;
  // Send speed to server
  fetch('/led1/fadespeed?speed=' + speed)
    .then(response => response.text())
    .then(data => console.log('Fade speed updated:', data))
    .catch(error => console.error('Error updating fade speed:', error));
}

function startFade() {
  fetch('/led1/fade?action=start')
    .then(response => response.text())
    .then(data => {
      console.log('Fade started:', data);
    })
    .catch(error => {
      console.error('Error starting fade:', error);
      alert('Error starting fade');
    });
}

function stopFade() {
  fetch('/led1/fade?action=stop')
    .then(response => response.text())
    .then(data => {
      console.log('Fade stopped:', data);
    })
    .catch(error => {
      console.error('Error stopping fade:', error);
      alert('Error stopping fade');
    });
}

// Page loaded
window.onload = function() {
  // Any initialization code can go here
};

