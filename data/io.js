// async function
async function fetchAsync(url) {
    // await response of fetch call
    let response = await fetch(url);
    // only proceed once promise is resolved
    let data = await response.json();
    // only proceed once second promise is resolved
    return data;
}

const sensorData = document.getElementById('sensorData');

function success(data) {
    console.log(data);
    sensorData.innerHTML = data;
  }
function error(reason) {
    console.log('Error: ', reason.message);
    sensorData.innerHTML = reason.message;
}

var intervalId = window.setInterval(function () {
    fetchAsync('getSensorData')
        .then(success)
        .catch(error)
}, 5000);