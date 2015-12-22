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
gpio.mode(2, gpio.OUTPUT)

srv = net.createServer(net.TCP)
srv:listen(80, function(conn)
  conn:on('sent', function(conn)
    conn:close()
  end)
  
  conn:on('receive', function(conn, payload)
    if payload:find('GET /') == 1 then
      conn:send('HTTP/1.0 200 OK\r\n\r\n' ..
        '<html><head><meta charset="utf-8"><title>ESP-8266</title></head><body>' ..
        '<input type="button" value="Toggle GPIO 1" onclick="x=new XMLHttpRequest();x.open(\'POST\', \'1-\'+(a=1-a));x.send()" />' ..
        '<input type="button" value="Toggle GPIO 2" onclick="x=new XMLHttpRequest();x.open(\'POST\', \'2-\'+(b=1-b));x.send()" />' ..
        '</body><script>a=0;b=0</script></html>')
    end
    
    if payload:find('POST /1%-0') == 1 then
      gpio.write(1, gpio.LOW)
      conn:send('HTTP/1.0 204 No Content\r\n\r\n')
    end
    
    if payload:find('POST /1%-1') == 1 then
      gpio.write(1, gpio.HIGH)
      conn:send('HTTP/1.0 204 No Content\r\n\r\n')
    end
    
    if payload:find('POST /2%-0') == 1 then
      gpio.write(2, gpio.LOW)
      conn:send('HTTP/1.0 204 No Content\r\n\r\n')
    end
    
    if payload:find('POST /2%-1') == 1 then
      gpio.write(2, gpio.HIGH)
      conn:send('HTTP/1.0 204 No Content\r\n\r\n')
    end
  end)
end)
