setInterval(function ( ) {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById("temperature").innerHTML = this.responseText;
        }
        };
        xhttp.open("GET", "/temperature", true);
        xhttp.send();
    }, 3000 
);

function goConfig() {
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/config", true);
    xhr.send();
}