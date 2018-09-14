const config = {
    baseUrl: "http://192.168.187.248" /* UNCOMMENT THIS LINE BEFORE SENT TO PRODUCTION */
};

function toggleSwitch(id) {
    const someUrl = config.baseUrl + "/toggle-switch?id=" + id;
    $.ajax({
        type: "POST",
        url: someUrl,
        contentType: "text/plain; charset=utf-8",
        dataType: "json",
        success: function (response) {

        },
        error: function () {

        }, complete: function () {

        },
        timeout: 2000
    });
}


function findNetworks() {
    $('#networks').empty();
    $('#status-scan').text('a pesquisar, aguarde...');
    const someUrl = config.baseUrl + "/scan";
    $.ajax({
        url: someUrl,
        contentType: "text/plain; charset=utf-8",
        dataType: "json",
        success: function (response) {

        },
        error: function () {

        }, complete: function () {

        },
        timeout: 2000
    });
}

function addZeros(i) {
    return i < 10 ? "0" + i : i
}

function loadConfig() {
    const someUrl = config.baseUrl + "/config";
    $.ajax({
        url: someUrl,
        contentType: "text/plain; charset=utf-8",
        dataType: "json",
        success: function (response) {
            localStorage.setItem("config", JSON.stringify(response));
            fillConfig();
        },
        error: function () {

        }, complete: function () {

        },
        timeout: 2000
    });
}

function loadSwitchs() {
    const someUrl = config.baseUrl + "/switchs";
    $.ajax({
        url: someUrl,
        contentType: "text/plain; charset=utf-8",
        dataType: "json",
        success: function (response) {
            refreshDashboard(response);

        },
        error: function () {

        }, complete: function () {

        },
        timeout: 2000
    });
}

function fillConfig() {
    let response = JSON.parse(localStorage.getItem("config"));
    $('input[name="nodeId"]').val(response.nodeId);
    $('input[name="mqttIpDns"]').val(response.mqttIpDns);
    $('input[name="mqttUsername"]').val(response.mqttUsername);
    $('input[name="mqttPassword"]').val(response.mqttPassword);
    $('input[name="wifiSSID"]').val(response.wifiSSID);
    $('input[name="wifiSecret"]').val(response.wifiSecret);
    $("#firmwareVersion").text(response.firmwareVersion);
    $("#version_lbl").text(response.firmwareVersion);
    $('#ff').prop('disabled', false);
}

function toggleActive(menu) {
    $('.sidebar-menu').find('li').removeClass('active');
    $('.menu-item[data-menu="' + menu + '"]').closest('li').addClass('active');
    $(".content").load(menu + ".html", function () {
        fillConfig();
        if (menu === "dashboard") {
            loadSwitchs();
        }


    });

}

function refreshDashboard(payload) {
    if (!payload) return;
    $('#devices').empty();
    for (let obj of payload) {
        $('#devices').append('<div class="col-lg-4 col-md-6 col-xs-12"><div class="info-box bg-aqua"><span class="info-box-icon"><i id=icon_' + obj["id"] + '  class="fa ' + obj["icon"] + ' ' + obj["stateControl"] + '"></i></span><div class="info-box-content"><span class="info-box-text">' + obj["name"] + '</span> <i id=btn_' + obj["id"] + ' class="fa fa-3x fa-toggle-on  toggler"></i></div></div></div>');
        $('#icon_' + obj["id"]).addClass(obj["stateControl"] ? 'on' : 'off');
        $('#btn_' + obj["id"]).addClass(obj["stateControl"] ? '' : 'fa-rotate-180');
        $('#btn_' + obj["id"]).on('click', function () {
            toggleSwitch(obj["id"]);
        });
    }
}

function updateSwitch(obj) {
    if (!obj) return;
    console.log(obj["stateControl"]);
    $('#icon_' + obj["id"]).removeClass('on').removeClass('off');
    $('#icon_' + obj["id"]).addClass(obj["stateControl"] ? 'on' : 'off');
    $('#btn_' + obj["id"]).removeClass('fa-rotate-180');
    $('#btn_' + obj["id"]).addClass(obj["stateControl"] ? '' : 'fa-rotate-180');
}

function wifiStatus(response) {
    $('#ssid_lbl').text(response.wifiSSID);
    $('#wifi-icon').attr('title', response.wifiSSID);
    if (response.status) {
        $('#wifi_status').text('ligado');
        $('#wifi_status_icon').removeClass('text-danger').addClass('text-ok');
        var percentage = Math.min(2 * (parseInt(response.signal) + 100), 100);
        $('#wifi-signal').text(percentage + "%");
        if (percentage > 0 && percentage < 30) {
            $('#wifi-icon')
                .removeClass('signal-zero')
                .removeClass('signal-med')
                .removeClass('signal-good')
                .removeClass('signal-bad')
                .addClass('signal-bad');
        } else if (percentage >= 30 && percentage < 61) {
            $('#wifi-icon')
                .removeClass('signal-zero')
                .removeClass('signal-med')
                .removeClass('signal-good')
                .removeClass('signal-bad')
                .addClass('signal-med');
        } else if (percentage >= 61) {
            $('#wifi-icon')
                .removeClass('signal-zero')
                .removeClass('signal-med')
                .removeClass('signal-good')
                .removeClass('signal-bad')
                .addClass('signal-good');
        }

    } else {
        $('#wifi_status_icon').removeClass('text-ok').addClass('text-danger')

        $('#wifi_status').text('desligado');
    }

}

function clearLog() {
    localStorage.setItem('log', "");
    refreshLogConsole();
}

function selectNetwork(node) {
    $('input[name="wifiSSID"]').val(node.split(": ")[5].trim());
    $('input[name="wifiSecret"]').val("").focus();

}

function appendNetwork(network) {
    if (network === "No networks found") {
        $('#status-scan').text('Não foram encontradas redes wi-fi');
    } else {
        $('#status-scan').text('resultado da pesquisa: ');
        $('#networks').append('<a href="#" class="wifi-node"  data-menu="' + network + '">' + network + '</a><p></p>');
        $('.wifi-node').click(function (e) {
            var node = $(e.currentTarget).data('menu');
            selectNetwork(node);

        });
    }
}

function refreshLogConsole() {
    $('#log-box').text(localStorage.getItem('log'))
}

function loadDefaults() {
    var someUrl = config.baseUrl + "/loaddefaults";
    $.ajax({
        url: someUrl,
        contentType: "text/plain; charset=utf-8",
        dataType: "json",
        success: function (response) {
        },
        timeout: 2000
    });
}

function getTimestamp() {
    let date = new Date();
    return date.toLocaleDateString('en-GB') + " " + addZeros(date.getHours()) + ":" + addZeros(date.getMinutes()) + ":" + addZeros(date.getSeconds());
}

function reboot() {
    var someUrl = config.baseUrl + "/reboot";
    $.ajax({
        url: someUrl,
        contentType: "text/plain; charset=utf-8",
        dataType: "json",
        success: function (response) {

        },
        timeout: 2000
    });
}

$(document).ready(function () {
    loadConfig();
    if (!!window.EventSource) {
        const source = new EventSource(config.baseUrl + '/events');
        source.addEventListener('open', function (e) {
            console.log("Events Connected");
            localStorage.setItem("last-update", "Atualizado em " + getTimestamp());
            refreshDashboard();
            refreshLogConsole();
        }, false);

        source.addEventListener('error', function (e) {
            if (e.target.readyState !== EventSource.OPEN) {
                console.log("Events Disconnected");
            }
        }, false);


        source.addEventListener('switch', function (e) {
            updateSwitch(JSON.parse(e.data));

        }, false);

        source.addEventListener('wifi', function (e) {
            wifiStatus(JSON.parse(e.data));
        }, false);

        source.addEventListener('wifi-networks', function (e) {
            appendNetwork(e.data);
        }, false);
        source.addEventListener('log', function (e) {

            let lastlog = localStorage.getItem("log") === null ? "" : localStorage.getItem("log");
            localStorage.setItem("log", getTimestamp() + "\n " + e.data + "\n\n" + lastlog);
            refreshLogConsole();
            console.log(e.data);
        }, false);
    }

    $('#node_id').on('keypress', function (e) {
        if (e.which === 32)
            return false;
    });


    $('.menu-item').click(function (e) {
        var menu = $(e.currentTarget).data('menu');
        toggleActive(menu);

    });

    toggleActive("dashboard");


});
