<!DOCTYPE html>
<html lang="en">
    <head>
        <meta charset="UTF-8">
        <title>Arase Home Raspberry Pi Server</title>
        <meta name="description" contents="Links to other services">
<!--        <link rel="stylesheet" href="css/style.css" type="text/css">-->
    </head>
    <body onload="init();">
        <header>
            <span id="current-time">
                <?php echo date('Y-m-d H:i:s'); ?>
            </span>
            <nav id="main-navigation">
                <ul>
                    <?php $raspi = getHostByName('raspberrypi.local.');
                          $ahms = getHostByName('arase-desktop.local.'); ?>
                    <li><a href="<?='http://'.$raspi.':80'?>">This page</a></li>
                    <li><a href="<?='http://'.$raspi.':3000'?>">Grafana</a></li>
                    <li><a href="<?='http://'.$raspi.':8888'?>">Chronograf</a></li>
                    <li><a href="<?='http://'.$raspi.':1880'?>">Node-RED</a></li>
                    <li><a href="<?='http://'.$ahms.':8096/web/index.html#!/home.html'?>">Emby</a></li>
                    <li><a href="<?='http://'.$raspi.':8080'?>">OpenHab</a></li>
                    <li><a href="<?='http://'.$raspi.':8200'?>">MiniDLNA</a></li>
                    <li><a href="phpinfo.php">PHP Info</a></li>
                </ul>
            </nav>
        </header>
        <div id="main-contents">
        </div>
        <footer>
            Built by $vbZ3r0
        </footer>
    </body>
</html>

<script type=text/javascript>
    function init(){
        updateTime();
        window.setInterval(updateTime, 1000);
    }

    function updateTime(){
        var time = document.getElementById('current-time');
        time.innerText = new Date().toLocaleString('en-IN', {timezone: 'IST'});
    }
</script>
