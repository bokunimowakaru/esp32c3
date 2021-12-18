/*******************************************************************************
HTMLコンテンツ LEDの輝度制御

                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/

void getHtml(char *html, int target){
    char s[65],s_ip[16];
    uint32_t ip = WiFi.localIP();

    sprintf(s_ip,"%d.%d.%d.%d",
        ip & 255,
        ip>>8 & 255,
        ip>>16 & 255,
        ip>>24
    );
    snprintf(html,2047,"<html>\n<head>\n<title>Wi-Fi コンシェルジェ 照明担当</title>\n<meta http-equiv=\"Content-type\" content=\"text/html; charset=UTF-8\">\n</head>\n<body>\n<h3>LED STATUS</h3>\n");
    if(target==0) sprintf(s,"<p>0 (LED OFF)</p>");
    if(target==1) sprintf(s,"<p>1 (LED ON)</p>");
    if(target>1)  sprintf(s,"<p>%d (輝度=%d)</p>",target);
    snprintf(html,2047,"%s\n%s\n<hr>\n<h3>HTTP GET</h3>\n<p>http://%s/?L=n<br>\n(n: 0=OFF, 1=ON)</p>",html,s,s_ip);
    sprintf(s,"<form method=\"GET\" action=\"http://%s/\">",s_ip);
    snprintf(html,2047,"%s\n%s\n<input type=\"submit\" name=\"L\" value=\"0 (OFF)\">\n<input type=\"submit\" name=\"L\" value=\"1 (ON)\">\n</form>\n",html,s);
    snprintf(html,2047,"%s\n%s\n<input type=\"text\" name=\"L\" value=\"\">\n<input type=\"submit\" value=\"送信\">\n</form>\n</body>\n</html>",html,s);
}
