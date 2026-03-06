function showDialogWindow(bx, by, out_close) {
    if (bx == undefined || bx == null) bx = 600;
    if (by == undefined || by == null) by = 440;
    if (out_close == undefined || out_close == null) out_close = true;

    var left = parseInt(bx / 2, 10);
    var top = parseInt(by / 2, 10);

    if (document.getElementById("_hidden_frame") == null) {
        $(
            '<div id="_hidden_frame" style="position:fixed; left:0; top:0; width:100%; height:100%; background:#f2f2f2; z-index:11; opacity:0.35"></div>'
        ).appendTo("body");
    }
    $("#_hidden_frame").show();
    if (out_close) {
        $("#_hidden_frame").click(function () {
            hideDialogWindow();
        });
    }

    if (document.getElementById("userModal") != null) {
        $("#userModal").css({ "margin-left": -left + "px", "margin-top": -top + "px" });
        $("#userModal").width(bx);
        $("#userModal").height(by);
        $("#userModal").fadeIn("fast");
    }
}

function hideDialogWindow() {
    if (document.getElementById("userModal")) $("#userModal").hide();
    $("#_hidden_frame").hide();
}

function showImageDialogWindow(bx, by, out_close) {
    if (bx == undefined || bx == null) bx = 600;
    if (by == undefined || by == null) by = 440;
    if (out_close == undefined || out_close == null) out_close = true;

    var left = parseInt(bx / 2, 10);
    var top = parseInt(by / 2, 10);

    if (document.getElementById("_hidden_frame2") == null) {
        $(
            '<div id="_hidden_frame2" style="position:fixed; left:0; top:0; width:100%; height:100%;  background:#f2f2f2; z-index:31; opacity:0.35"></div>'
        ).appendTo("body");
    }
    $("#_hidden_frame2").show();
    if (out_close) {
        $("_hidden_frame2").click(function () {
            hideImageDialogWindow();
        });
    }

    if (document.getElementById("imgModal") != null) {
        $("#imgModal").css({ "margin-left": -left + "px", "margin-top": -top + "px" });
        $("#imgModal").width(bx);
        $("#imgModal").height(by);
        $("#imgModal").fadeIn("fast");
    }
}

function hideImageDialogWindow() {
    if (document.getElementById("imgModal")) $("#imgModal").hide();
    $("#_hidden_frame2").hide();
}
