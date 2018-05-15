Add-Type -Path 'C:\Users\jtutt\OneDrive\bin\NuGet\M2Mqtt.4.3.0.0\lib\net45\M2Mqtt.Net.dll'
$MqttClient = [uPLibrary.Networking.M2Mqtt.MqttClient]("service.joerg-tuttas.de")
#
# Verbinden
$mqttclient.Connect([guid]::NewGuid())
Register-ObjectEvent -inputObject $MqttClient -EventName MqttMsgPublishReceived -Action {Write-host "Event Found Topic: $($args[1].topic)  Message $([System.Text.Encoding]::ASCII.GetString($args[1].Message))"}
#$mqttClient.Subscribe("esp32/temp",0)
$msg = "" | Select-Object "text","username","screen_name"
$msg.text="Hallo Welt"
$msg.username="Powershell"
$msg.screen_name="Powershell"
$s=ConvertTo-Json $msg
$MqttClient.Publish("mmbbs/hack", [System.Text.Encoding]::UTF8.GetBytes($s))