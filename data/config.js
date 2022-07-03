var wifi_dialog = document.querySelector('#delete-wifi');
    var del_wbtn = document.querySelector('#del-wifi_btn');
    if (! wifi_dialog.showModal) {
      dialogPolyfill.registerDialog(wifi_dialog);
    }
    del_wbtn.addEventListener('click', function() {
        wifi_dialog.showModal();
    });
    wifi_dialog.querySelector('.close').addEventListener('click', function() {
        wifi_dialog.close();
});

var cfg_dialog = document.querySelector('#delete-cfg');
    var del_wbtn = document.querySelector('#del-cfg_btn');
    if (! cfg_dialog.showModal) {
      dialogPolyfill.registerDialog(cfg_dialog);
    }
    del_wbtn.addEventListener('click', function() {
        cfg_dialog.showModal();
    });
    cfg_dialog.querySelector('.close').addEventListener('click', function() {
        cfg_dialog.close();
});

function goHome() {
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/", true);
    xhr.send();
}
