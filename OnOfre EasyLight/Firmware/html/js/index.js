const endpoint = {
    baseUrl: "" /* UNCOMMENT THIS LINE BEFORE SENT TO PRODUCTION */
};

function toggleSwitch(id) {
    const someUrl = endpoint.baseUrl + "/toggle-switch?id=" + id;
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

function storedevice(id, _device, endpointstore, endointget, func) {
    const someUrl = endpoint.baseUrl + "/"+endpointstore+"?id=" + id;
    $.ajax({
        type: "POST",
        url: someUrl,
        dataType: "json",
        contentType: "application/json",
        data: JSON.stringify(_device),
        success: function (response) {
            loadDevice(func, endointget);
            alert("Configuração Guardada");
        },
        error: function () {
            alert("Erro não foi possivel guardar a configuração");
        }, complete: function () {

        },
        timeout: 2000
    });
}
function storeConfig(path,newConfig) {
    const someUrl = endpoint.baseUrl + "/"+path;
    $.ajax({
        type: "POST",
        url: someUrl,
        dataType: "json",
        contentType: "application/json",
        data: JSON.stringify(newConfig),
        success: function (response) {
            alert("Configuração Guardada");
        },
        error: function () {
            alert("Erro não foi possivel guardar a configuração");

        }, complete: function () {

        },
        timeout: 2000
    });
}

function findNetworks() {
    $('#networks').empty();
    $('#status-scan').text('a pesquisar, aguarde...');
    const someUrl = endpoint.baseUrl + "/scan";
    $.ajax({
        url: someUrl,
        contentType: "text/plain; charset=utf-8",
        dataType: "json",
        success: function (response) {

        },
        error: function () {
            alert("Erro não foi possivel fazer o pedido");
        }, complete: function () {

        },
        timeout: 2000
    });
}

function addZeros(i) {
    return i < 10 ? "0" + i : i
}

function loadConfig() {
    const someUrl = endpoint.baseUrl + "/config";
    $.ajax({
        url: someUrl,
        contentType: "text/plain; charset=utf-8",
        dataType: "json",
        success: function (response) {
            localStorage.setItem("config", JSON.stringify(response));
            fillConfig();
        },
        error: function () {
            alert("Erro a carregar configuração");
        }, complete: function () {

        },
        timeout: 2000
    });
}


function loadDevice(func, e) {
    const someUrl = endpoint.baseUrl + "/" + e;
    $.ajax({
        url: someUrl,
        contentType: "text/plain; charset=utf-8",
        dataType: "json",
        success: function (response) {
            func(response);

        },
        error: function () {
            alert("Erro a carregar configuração dos dispositivos");
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
    $('select[name="homeAssistantAutoDiscovery"] option[value="' + response.homeAssistantAutoDiscovery + '"]').attr("selected", "selected");
    $('input[name="homeAssistantAutoDiscoveryPrefix"]').val(response.homeAssistantAutoDiscoveryPrefix);
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
          loadDevice(refreshDashboard, "switchs");
        } else if (menu === "devices") {
            loadDevice(fillSwitchs, "switchs");
            loadDevice(fillRelays, "relays")
        }

    });
}


function fillSwitchs(payload) {
    if (!payload) return;
    $('#switch_config').empty();
    for (let obj of payload) {

        $('#switch_config').append("<div class=\"col-lg-4 col-md-6 col-xs-12\">" +
            "        <div style=\"margin-bottom: 0px\" class=\"info-box bg-aqua\"><span class=\"info-box-icon\">" +
            "        <i id=\"icon_" + obj.id + "\" class=\"fa " + obj.icon + " false\"></i></span>" +
            "            <div class=\"info-box-content\"><span class=\"info-box-text\">" + obj.name + "</span>" +
            "                <i id=\"btn_" + obj.id + "\" style=\"float: right\" class=\"fa fa-3x fa-toggle-on toggler \"></i>" +
            "            </div>" +
            "        </div>" +
            "        <div style=\"font-size: 10px;  border: 0px solid #08c; border-radius: 0\" class=\"box\">" +
            "            <div class=\"box-body no-padding\">" +
            "                <table class=\"table table-condensed\">" +
            "                    <tbody>" +
            "                    <tr>" +
            "                        <td><span style=\"font-size: 10px;width: 100px;\" class=\"badge bg-blue\">NOME</span></td>" +
            "                        <td><input  style=\"font-size: 10px; height: 20px;\"  class=\"form-control\" value=\"" + obj.name + "\" type=\"text\"  id=\"name_" + obj.id + "\" placeholder=\"ex: luz sala\"  required=\"true\"/></td>" +
            "                    </tr>" +
            "                    <tr>" +
            "                        <td><span style=\"font-size: 10px;width: 100px;\" class=\"badge bg-blue\">GPIO</span></td>" +
            "                        <td><select class=\"form-control\" style=\"font-size: 10px; padding: 0px 12px; height: 20px;\"" +
            "                                    id=\"gpio_" + obj.id + "\">" +
            "                            <option value=\"" + obj.gpio + "\">" + obj.gpio + "</option>" +
            "                        </select></td>" +
            "                    </tr>" +
            "                    <tr>" +
            "                        <td><span style=\"font-size: 10px;width: 100px;\" class=\"badge bg-blue\">PULLUP</span></td>" +
            "                        <td><select class=\"form-control\" style=\"font-size: 10px; padding: 0px 12px; height: 20px;\"" +
            "                                     id=\"pullup_" + obj.id + "\">" +
            "                            <option " + (obj.pullup ? 'selected' : '') + " value=\"true\">Sim</option>" +
            "                            <option " + (!obj.pullup ? 'selected' : '') + " value=\"false\">Não</option>" +
            "                        </select></td>" +
            "                    </tr>" +
            "                    <tr>" +
            "                        <td><span style=\"font-size: 10px;width: 100px;\" class=\"badge bg-blue\">MODO</span></td>" +
            "                        <td>" +
            "" +
            "                            <select class=\"form-control\" style=\"font-size: 10px; padding: 0px 12px; height: 20px;\"" +
            "                                     id=\"mode_" + obj.id + "\">" +
            "                                <option " + (obj.mode == 1 ? 'selected' : '') + " value=\"1\">INTERRUPTOR COMUM</option>" +
            "                                <option " + (obj.mode == 2 ? 'selected' : '') + " value=\"2\">INTERRUPTOR PUSH</option>" +
            "                            </select>" +
            "" +
            "                        </td>" +
            "                    </tr>" +
            "                    <tr>" +
            "                        <td><span style=\"font-size: 10px;width: 100px;\" class=\"badge bg-blue\">COMUTA</span></td>" +
            "                        <td><div class=\"row\">" +
            "                <div class=\"col-xs-5\">" +
            "                        <select class=\"form-control\" style=\"font-size: 10px;  padding: 0px 12px; height: 20px;\"" +
            "                                 id=\"typeControl_" + obj.id + "\">" +
            "                            <option " + (obj.typeControl === "relay" ? 'selected' : '') + " value=\"relay\">RELÉ</option>" +
            "                        </select>" +
            "                        </select>" +
            "                </div>" +
            "                <div class=\"col-xs-2\">ID/GPIO" +
            "                </div>" +
            "                <div class=\"col-xs-5\">" +
            "                           <select class=\"form-control\" style=\" font-size: 10px;padding: 0px 12px; height: 20px;\"" +
            "                                 id=\"gpioControl_" + obj.id + "\">" +
            "                            <option " + (obj.gpioControl == 5 ? 'selected' : '') + " value=\"5\">5</option>" +
            "                            <option " + (obj.gpioControl == 4 ? 'selected' : '') + " value=\"4\">4</option>" +
            "                        </select>" +
            "                </div>" +
            "              </div>" +


            "</td>" +
            "                    </tr>" +
            "                    <tr>" +
            "                        <td><span style=\"font-size: 10px;width: 100px;\" class=\"badge bg-blue\">MESTRE</span></td>" +
            "                        <td><select class=\"form-control\" style=\"font-size: 10px; padding: 0px 12px; height: 20px;\"" +
            "                                     id=\"master_" + obj.id + "\">" +
            "                            <option " + (!obj.master ? 'selected' : '') + " value=\"true\">Sim</option>" +
            "                            <option " + (obj.master ? 'selected' : '') + " value=\"false\">Não</option>" +
            "                        </select></td>" +
            "                    </tr>" +
            "                    <tr>" +
            "                        <td><span style=\"font-size: 10px;width: 100px;\" class=\"badge bg-blue\">MQTT ESTADO</span></td>" +
            "                        <td><span style=\"font-weight: bold; font-size:11px; color: #00a65a\">" + obj.mqttStateTopic + "</span>" +
            "                        </td>" +
            "" +
            "                    </tr>" +
            "                    <tr>" +
            "                        <td><span style=\"font-size: 10px;width: 100px;\" class=\"badge bg-blue\">MQTT CONTROLO</span></td>" +
            "                        <td><span style=\"font-weight: bold; font-size:11px; color:#f39c12\">" + obj.mqttCommandTopic + "</span>" +
            "                        </td>" +
            "" +
            "                    </tr>" +
            "                    </tbody>" +
            "                </table>" +
            "                <div class=\"box-footer save\">" +
            "                    <button onclick=\"saveSwitch(" + obj.id + ")\" style=\"font-size: 12px\" class=\"btn btn-primary\">Guardar</button>" +
            "                </div>" +
            "            </div>" +
            "        </div>" +
            "" +
            "    </div>");
        $('#icon_' + obj["id"]).addClass(obj["stateControl"] ? 'on' : 'off');
        $('#btn_' + obj["id"]).addClass(obj["stateControl"] ? '' : 'fa-rotate-180');
        $('#btn_' + obj["id"]).on('click', function () {
            toggleSwitch(obj["id"]);
        });
    }
}

function fillRelays(payload) {
    if (!payload) return;
    $('#relay_config').empty();
    for (let obj of payload) {
        $('#relay_config').append("<div class=\"col-lg-4 col-md-6 col-xs-12\">" +
            "        <div style=\"margin-bottom: 0px\" class=\"info-box bg-aqua\"><span class=\"info-box-icon\">" +
            "        <i id=\"icon_" + obj.id + "\" class=\"fa " + obj.icon + " false off\"></i></span>" +
            "            <div class=\"info-box-content\"><span class=\"info-box-text\">" + obj.name + "</span>" +
            "            </div>" +
            "        </div>" +
            "        <div style=\"font-size: 10px; border-radius: 0\" class=\"box\">" +
            "            <div class=\"box-body no-padding\">" +
            "                <table class=\"table table-condensed\">" +
            "                    <tbody>" +
            "                    <tr>" +
            "                        <td><span style=\"font-size: 10px;width: 100px;\" class=\"badge bg-blue\">NOME</span></td>" +
            "                        <td><input  style=\"font-size: 10px; height: 20px;\"  class=\"form-control\" value=\"" + obj.name + "\" type=\"text\"  id=\"name_" + obj.id + "\" placeholder=\"ex: luz sala\"  required=\"true\"/></td>" +
            "                    </tr>" +
            "                    <tr>" +
            "                        <td><span style=\"font-size: 10px;width: 100px;\" class=\"badge bg-blue\">GPIO</span></td>" +
            "                        <td><select class=\"form-control\" style=\"font-size: 10px; padding: 0px 12px; height: 20px;\"" +
            "                                    id=\"gpio_" + obj.id + "\">" +
            "                            <option value=\"" + obj.gpio + "\">" + obj.gpio + "</option>" +
            "                        </select></td>" +
            "                    </tr>" +
            "                    <tr>" +
            "                        <td><span style=\"font-size: 10px;width: 100px;\" class=\"badge bg-blue\">INVERTIDO</span></td>" +
            "                        <td><select class=\"form-control\" style=\"font-size: 10px; padding: 0px 12px; height: 20px;\"" +
            "                                     id=\"inverted_" + obj.id + "\">" +
            "                            <option " + (obj.inverted ? 'selected' : '') + " value=\"true\">Sim</option>" +
            "                            <option " + (!obj.inverted ? 'selected' : '') + " value=\"false\">Não</option>" +
            "                        </select></td>" +
            "                    </tr>" +
            "                    </tbody>" +
            "                </table>" +
            "                <div class=\"box-footer save\">" +
            "                    <button onclick=\"saveRelay(" + obj.id + ")\" style=\"font-size: 12px\" class=\"btn btn-primary\">Guardar</button>" +
            "                </div></div></div></div>");
    }
}

function saveSwitch(id) {
    var device = {
        "name": $('#name_' + id).val(),
        "gpio": $('#gpio_' + id).val(),
        "pullup": $('#pullup_' + id).val(),
        "mode": $('#mode_' + id).val(),
        "typeControl": $('#typeControl_' + id).val(),
        "gpioControl": $('#gpioControl_' + id).val(),
        "master": $('#master_' + id).val()
    };

    storedevice(id, device, "save-switch","switchs",fillSwitchs);
}

function saveRelay(id) {
    var device = {
        "name": $('#name_' + id).val(),
        "gpio": $('#gpio_' + id).val(),
        "inverted": $('#inverted_' + id).val()
    };

    storedevice(id, device, "save-relay","relays",fillRelays);

}

function saveNode() {
    var _config = {
        "nodeId": $('#nodeId').val(),

    };
    storeConfig("save-node",_config);
}
function saveWifi() {
    var _config = {
        "wifiSSID": $('#ssid').val(),
        "wifiSecret": $('#wifi_secret').val()

    };
    storeConfig("save-wifi",_config);
}
function saveMqtt() {
    var _config = {
        "mqttIpDns": $('#mqtt_ip').val(),
        "mqttUsername": $('#mqtt_username').val(),
        "mqttPassword": $('#mqtt_password').val()

    };
    storeConfig("save-mqtt",_config);
}
function saveHa() {
    var _config = {
        "homeAssistantAutoDiscovery": $('#homeAssistantAutoDiscovery').val(),
        "homeAssistantAutoDiscoveryPrefix": $('#homeAssistantAutoDiscoveryPrefix').val()

    };
    storeConfig("save-ha",_config);
}

function refreshDashboard(payload) {
    if (!payload) return;
    $('#devices').empty();
    let a = ""
    for (let obj of payload) {
        $('#devices').append('<div class="col-lg-4 col-md-6 col-xs-12"><div class="info-box bg-aqua"><span class="info-box-icon"><i id=icon_' + obj["id"] + '  class="fa ' + obj["icon"] + ' ' + obj["stateControl"] + '"></i></span><div class="info-box-content"><span class="info-box-text">' + obj["name"] + '</span> <i id=btn_' + obj["id"] + ' style="float: right" class="fa fa-3x fa-toggle-on  toggler"></i></div></div></div>');
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
    var someUrl = endpoint.baseUrl + "/loaddefaults";
    $.ajax({
        url: someUrl,
        contentType: "text/plain; charset=utf-8",
        dataType: "json",
        success: function (response) {
            alert("Configuração de fábrica aplicada com sucesso. Por favor volte a ligar-se ao Access Point de configuração e aceda ao painel de controlo pelo endereço http://192.168.4.1 no seu browser.");
        },
        timeout: 2000
    });
}

function getTimestamp() {
    let date = new Date();
    return date.toLocaleDateString('en-GB') + " " + addZeros(date.getHours()) + ":" + addZeros(date.getMinutes()) + ":" + addZeros(date.getSeconds());
}

function reboot() {
    var someUrl = endpoint.baseUrl + "/reboot";
    $.ajax({
        url: someUrl,
        contentType: "text/plain; charset=utf-8",
        dataType: "json",
        success: function (response) {
            alert("O OnoFre está a reiniciar, ficará disponivel dentro de 10 segundos.");
        },
        timeout: 2000
    });
}

$(document).ready(function () {
    loadConfig();
    if (!!window.EventSource) {
        const source = new EventSource(endpoint.baseUrl + '/events');
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
            localStorage.setItem("log", getTimestamp() + " " + e.data + "" + lastlog);
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