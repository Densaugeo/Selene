print('init.lua ver 1.2')
print('Chip: ', node.chipid())
print('Heap: ', node.heap())

-- Wifi settings
cfg = {ssid = "ESP-test", pwd = "ESP-test"}
wifi.setmode(wifi.SOFTAP)
wifi.ap.config(cfg)

print('Set mode=STATION (mode='..wifi.getmode()..')')
print('MAC: ', wifi.sta.getmac())
address = {
  ip = '192.168.4.1',
  netmask = '255.255.255.0',
  gateway = '192.168.4.1'
}
wifi.ap.setip(address)

print('IP: ', wifi.ap.getip())

-- Initialize GPIO (pin 4 is GPIO2 on my ESP-01)
gpio.mode(1, gpio.OUTPUT)
pwm.setup(2, 100, 0)
gpio.mode(0, gpio.OUTPUT)
pwm.setup(8, 100, 0)

srv = net.createServer(net.TCP)
srv:listen(80, function(conn)
  conn:on('sent', function(conn)
    conn:close()
  end)
  
  conn:on('receive', function(conn, payload)
    if payload:find('GET /') == 1 then
      conn:send('HTTP/1.0 200 OK\r\n\r\n' ..
        '<title>ESP-8266</title><body style="background:#000;color:#F00">' ..
        '<input type="range" min="-1023" max="1023" value="0" orient="vertical" style="height:100%;float:left" onchange="x=new XMLHttpRequest();x.open(\'POST\',\'1=\'+this.value);x.send()">' ..
        '<input type="range" min="-1023" max="1023" value="0" orient="vertical" style="height:100%;float:right" onchange="x=new XMLHttpRequest();x.open(\'POST\',\'2=\'+this.value);x.send()">' ..
        '<h1 style="float:left" onclick="x=new XMLHttpRequest();x.open(\'POST\',\'1=0\');x.send()">Stop</h1>' ..
        '<h1 style="float:right" onclick="x=new XMLHttpRequest();x.open(\'POST\',\'2=0\');x.send()">Stop</h1>')
    end
    
    if payload:find('POST /1=%-') == 1 then
      gpio.write(1, gpio.HIGH)
      pwm.setduty(2, 1023 - payload:match("POST /1=%-(%d+)"))
      conn:send('HTTP/1.0 204 No Content\r\n\r\n')
    end
    
    if payload:match("POST /1=(%d+)") ~= nil then
      gpio.write(1, gpio.LOW)
      pwm.setduty(2, payload:match("POST /1=(%d+)"))
      conn:send('HTTP/1.0 204 No Content\r\n\r\n')
    end
    
    if payload:find('POST /2=%-') == 1 then
      gpio.write(0, gpio.HIGH)
      pwm.setduty(8, 1023 - payload:match("POST /2=%-(%d+)"))
      conn:send('HTTP/1.0 204 No Content\r\n\r\n')
    end
    
    if payload:match("POST /2=(%d+)") ~= nil then
      gpio.write(0, gpio.LOW)
      pwm.setduty(8, payload:match("POST /2=(%d+)"))
      conn:send('HTTP/1.0 204 No Content\r\n\r\n')
    end
  end)
end)
