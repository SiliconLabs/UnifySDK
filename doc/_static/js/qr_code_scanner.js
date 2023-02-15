$(document).ready(function() {
    $("#qr-code-scanner").append("<div id='qr-reader'></div><div class='error-message'></div><div class='dsk-container'><div style='display:inline-flex'><b>DSK: </b><div class='dsk-value'></div><button type='button' id='copy' class='btn btn-info'>Copy</button></div></div>");
    var scanner = new Html5QrcodeScanner("qr-reader", { fps: 10, qrbox: 250 });

    if (!/Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(navigator.userAgent))
        $("#qr-code-scanner")[0].classList.add('flip');

    function onScanSuccess(data, decodedResult) {
        if (data) {
            if (isNaN(data)) {
                $(".error-message").text("Incorrect Qr-Code format");
                $(".dsk-value").text('');
                $(".dsk-container")[0].style.visibility = 'hidden';
                return;
            }
            try {
                let sha = getSha1(data.slice(9));
                let shaArray = [];
                for (let index = 0; index < sha.length; index = index + 2) {
                    shaArray.push(parseInt("0x" + sha.slice(index, index + 2)));
                }
                let sha1CheckSum = (shaArray[0] << 8) + shaArray[1];
                let dataCheckSum = parseInt(data.slice(4, 4 + 5))
                if (sha1CheckSum !== dataCheckSum) {
                    $(".error-message").text("Checksum is incorrect");
                    $(".dsk-value").text('');
                    $(".dsk-container")[0].style.visibility = 'hidden';
                    return;
                }

                let dsk = data.slice(12, 12 + 40);
                $(".dsk-value").text(dsk);
                $(".error-message").text("");
                $(".dsk-container")[0].style.visibility = 'visible';
            } catch (error) {
                $(".error-message").text("Can't parse DSK");
                $(".dsk-value").text('');
                $(".dsk-container")[0].style.visibility = 'hidden';
                return;
            }
        }
    }

    function onScanError(errorMessage) {
        console.log(errorMessage)
            // handle on error condition, with error message
    }
    scanner.render(onScanSuccess);

    $("#copy").on('click', function(event) {
        navigator.clipboard.writeText($(".dsk-value").text());
    });
});