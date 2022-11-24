// async function
async function fetchAsync(url) {
    // await response of fetch call
    let response = await fetch(url);
    // only proceed once promise is resolved
    let data = await response.json();
    // only proceed once second promise is resolved
    return data;
}

const dataField = document.getElementById('data');
const buttonSwitch = document.getElementById('switch');

function dataSuccess(data) {
    //console.log(data);
    dataField.innerHTML = JSON.stringify(data, undefined, 2);
}

function dataError(reason) {
    //console.log('Error: ', reason.message);
    dataField.innerHTML = reason.message;
}

var intervalId = window.setInterval(function () {
    fetchAsync('data')
        .then(dataSuccess)
        .catch(dataError)
}, 500);


function switchResponse(data) {
    console.log(data);
}

function switchLed() {
    fetchAsync('switch')
        .then(switchResponse)
        .catch(switchResponse)
}

buttonSwitch.addEventListener("click", switchLed);