/*******************************************************************************
HTMLコンテンツ LEDの輝度制御

                                          Copyright (c) 2016-2021 Wataru KUNINO
*******************************************************************************/

String getHtml(int target){
    String S;
    char s[65],s_ip[16];
    uint32_t ip = WiFi.localIP();

    sprintf(s_ip,"%d.%d.%d.%d",
        ip & 255,
        ip>>8 & 255,
        ip>>16 & 255,
        ip>>24
    );
    S = "<html>\n<head>\n<title>Wi-Fi コンシェルジェ 照明担当</title>\n<meta http-equiv=\"Content-type\" content=\"text/html; charset=UTF-8\">\n</head>\n<body>\n<h3>LED STATUS</h3>\n";
    if(target==0) S += "<p>0 (LED OFF)</p>\n";
    if(target==1) S += "<p>1 (LED ON)</p>\n";
    if(target>1)  S += "<p>%d (輝度=" + String(target) + ")</p>\n";
    S += "<hr>\n<h3>HTTP GET</h3>\n<p>http://" + String(s_ip) + "/?L=n<br>\n(n: 0=OFF, 1=ON)</p>\n";
    sprintf(s,"<form method=\"GET\" action=\"http://%s/\">",s_ip);
    S += String(s) + "\n<input type=\"submit\" name=\"L\" value=\"0 (OFF)\">\n<input type=\"submit\" name=\"L\" value=\"1 (ON)\">\n</form>\n";
    S += String(s) + "\n<input type=\"text\" name=\"L\" value=\"\">\n<input type=\"submit\" value=\"送信\">\n</form>\n</body>\n</html>";
    return S;
}
