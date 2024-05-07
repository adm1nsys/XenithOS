# XenithOS
Operation system for boards. The Operating System allows you to use the board as a security key. 
<hr>
<h1>Boards Support:</h1>

<ul>
  <h3>ESP</h3>
<li><a href="#ESP8266_ideaspark_table">ESP 8266 ideaspark</a></li>
<a href="#ESP8266_pinout">Pinout</a><br>
<a href="#ESP8266_pinout">Conect to Computer</a>
<li><a href="#ESP8266_regular_table">ESP 8266 (regular)</a></li>
<li><a href="#ESP32_table">ESP 32</a></li>
    <h3>Rasberry Pi</h3>
<li><a href="#Rasberry_Pi_Pico_table">Rasberry Pi Pico</a></li>
<li><a href="#Rasberry_Pi_Zero_table">Rasberry Pi Zero</a></li>
</ul>

<!-- ESP8266 ideaspark table -->

<table id="ESP8266_ideaspark_table">
<tr>
  <th>ESP8266 (ideaspark)</th>
  <th>Connection Type</th>
  <th>Release Data</th>
</tr>
<tr>
  <td>Beta 1</td>
  <td>Type 1</td>
  <td>2024 5 May</td>
</tr>
</table>

<!-- ESP8266 regular table -->

<table id="ESP8266_regular_table">
<tr>
  <th>ESP8266 (regular)</th>
  <th>Connection Type</th>
  <th>Release Data</th>
</tr>
<tr>
  <td>❌</td>
  <td>❌</td>
  <td>❌</td>
</tr>
</table>

<!-- ESP32 table -->

<table id="ESP32_table">
<tr>
  <th>ESP32</th>
  <th>Connection Type</th>
  <th>Release Data</th>
</tr>
<tr>
  <td>❌</td>
  <td>❌</td>
  <td>❌</td>
</tr>
</table>

<!-- Rasberry Pi Pico table -->

<table id="Rasberry_Pi_Pico_table">
<tr>
  <th>Rasberry Pi Pico</th>
  <th>Connection Type</th>
  <th>Release Data</th>
</tr>
<tr>
  <td>❌</td>
  <td>❌</td>
  <td>❌</td>
</tr>
</table>

<!-- Rasberry Pi Zero table -->

<table id="Rasberry_Pi_Zero_table">
<tr>
  <th>Rasberry Pi Zero</th>
  <th>Connection Type</th>
  <th>Release Data</th>
</tr>
<tr>
  <td>❌</td>
  <td>❌</td>
  <td>❌</td>
</tr>
</table>


<h1>ESP 8266 ideaspark Flash and connection:</h1>
<h2>Beta 1</h2>
To install XenithOS or XenOS an setup it Use Arduino ide. 
<h3>To correct connect board to pc use this instruction (<a href="https://www.aliexpress.com/item/1005005242283189.html?spm=a2g0o.productlist.main.3.3168W6AxW6Axbw&algo_pvid=109215a9-0e73-441b-b943-0d5ab0e12670&algo_exp_id=109215a9-0e73-441b-b943-0d5ab0e12670-1&pdp_npi=4%40dis%21UAH%21187.67%21175.92%21%21%214.63%214.34%21%4021164c9c17147468046175847e05e1%2112000032335054938%21sea%21UA%210%21AB&curPageLogUid=6lCsG9jNPfbw&utparam-url=scene%3Asearch%7Cquery_from%3A">Sourse</a>):</h3>
<img src="https://github.com/adm1nsys/XenithOS/blob/main/ESP%208266/ideaspark/Assets%20to%20setup/setup.png?raw=true"></img>
<h3>Also look on pinout:
<img src="https://github.com/adm1nsys/XenithOS/raw/main/ESP%208266/ideaspark/Assets%20to%20setup/connection.webp"></img>
<h3>Connection:</h3>
D1 -> Button 1 (top)<br>
D2 -> Button 2 (bottom)<br>
1 Pin of buttom is D1/D2 and 2 Pin is G (Gnd).<br>
Flash -> Apply.<br>
Flash Button is on board and you do not need to do anything.<br>
<h3>Setup Software:</h3>
Coppy code to Arduino ide, read instruction on line 9, or read it here: <br>
1. Connect Wi-Fi (line 17-18).<br>
2. Write your key (line 26).<br>
3. Name Servise.  (line 34).<br>
4. You can change time server (line 21 or 22). <br>
<h3>Flash:</h3>
Connect correctly board, verify code and click on flash. Then wait.<br>
<h3>Navigation:</h3>
D1(pin) Button: Action top in menu or back to menu.<br>
D2(pin) Button: Action bottom in menu.<br>
Flash Button (on board): Action apply (open item in menu).<br>
<h3>Status Bar:</h3>
Bottom right you will see 2 icons:<br>
Left icon is time sync status.<br>
Right icon is wifi connection status.<br>
! Problem with time<br>
T No problem with time<br>
X No Wi-Fi connection<br>
W No problem with Wi-Fi<br>

