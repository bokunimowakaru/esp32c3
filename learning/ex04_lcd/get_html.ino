/*******************************************************************************
HTMLコンテンツ 液晶

                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/
#define _html_size 777 + 32 + 32 + 64

String getHtml(String rx){
    char html[_html_size],s[65],s_ip[16];
    uint32_t ip = WiFi.localIP();
    sprintf(s_ip,"%d.%d.%d.%d",
        ip & 255,
        ip>>8 & 255,
        ip>>16 & 255,
        ip>>24
    );
    snprintf(html,_html_size,"<html>\n<head><title>Wi-Fi コンシェルジェ 掲示板担当</title>\n");
    strcat(html,"<meta http-equiv=\"Content-type\" content=\"text/html; charset=UTF-8\">");
    strcat(html,"</head>");
    strcat(html,"<body>");
    strcat(html,"<h3>LCD STATUS</h3>");
    strcat(html,"<p>");
    rx.toCharArray(s, 64);
    strcat(html,s);
    strcat(html,"</p>");
    strcat(html,"<hr>");
    strcat(html,"<h3>HTTP GET</h3>");
    sprintf(html,"%s<p>http://%s/?TEXT=文字列</p>",html,s_ip);
    sprintf(s,"<form method=\"GET\" action=\"http://%s/\">",s_ip);
    sprintf(html,"%s%s<input type=\"submit\" name=\"TEXT\" value=\"Hello\"></form>",html,s);
    sprintf(html,"%s%s<input type=\"text\" name=\"TEXT\" value=\"\">",html,s);
    strcat(html,"<input type=\"submit\" value=\"送信\">");
    strcat(html,"</form>");
    strcat(html,"<hr>");
    strcat(html,"<h3>HTTP POST</h3>");
    sprintf(s,"<form method=\"POST\" action=\"http://%s/\">",s_ip);
    sprintf(html,"%s%s<input type=\"submit\" name=\"TEXT\" value=\"Hello\"></form>",html,s);
    sprintf(html,"%s%s<input type=\"text\" name=\"TEXT\" value=\"\">",html,s);
    strcat(html,"<input type=\"submit\" value=\"送信\">");
    strcat(html,"</form>");
    strcat(html,"</body>");
    strcat(html,"</html>");
    // Serial.println("sizeof(html)=" + String(strlen(html)+1));
    // 00:59:00.291 -> sizeof(html)=777
    // 00:59:00.291 -> ｴﾚｷｼﾞｬｯｸIoT CQpb
    return String(html);

    /*
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
    S += "<head><title>Wi-Fi コンシェルジェ 掲示板担当</title>";
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
    */
}
