/*******************************************************************************
HTMLコンテンツ 液晶

                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/

String getHtml(String rx){
    String S;
    char s[65],s_ip[16];
    uint32_t ip = WiFi.localIP();
    
    sprintf(s_ip,"%d.%d.%d.%d",
        ip & 255,
        ip>>8 & 255,
        ip>>16 & 255,
        ip>>24
    );
    S = "<html>";
    S += "<head><title>Wi-Fi コンシェルジェ 表示担当</title>";
    S += "<meta http-equiv=\"Content-type\" content=\"text/html; charset=UTF-8\">";
    S += "</head>";
    S += "<body>";
    S += "<h3>LCD STATUS</h3>";
    S += "<p>";
    S += rx;    
    S += "</p>";
    S += "<hr>";
    S += "<h3>HTTP GET</h3>";
    S += "<p>http://";
    S += String(s_ip);
    S += "/?TEXT=文字列</p>";
    sprintf(s,"<form method=\"GET\" action=\"http://%s/\">",s_ip);
    S += String(s);
    S += "<input type=\"submit\" name=\"TEXT\" value=\"Hello\">";
    S += "</form>";
    S += String(s);
    S += "<input type=\"text\" name=\"TEXT\" value=\"\">";
    S += "<input type=\"submit\" value=\"送信\">";
    S += "</form>";
    S += "<hr>";
    S += "<h3>HTTP POST</h3>";
    sprintf(s,"<form method=\"POST\" action=\"http://%s/\">",s_ip);
    S += String(s);
    S += "<input type=\"submit\" name=\"TEXT\" value=\"Hello\">";
    S += "</form>";
    S += String(s);
    S += "<input type=\"text\" name=\"TEXT\" value=\"\">";
    S += "<input type=\"submit\" value=\"送信\">";
    S += "</form>";
    S += "</body>";
    S += "</html>";
    return S;
}
