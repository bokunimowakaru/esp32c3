/*******************************************************************************
HTMLコンテンツ IRリモコン

                                          Copyright (c) 2016-2022 Wataru KUNINO
*******************************************************************************/
#ifndef AEHA
#define AEHA        0
#define NEC         1
#define SIRC        2
#endif
#ifndef AUTO
#define AUTO        255
#endif

#define _html_size 914     // 874 + 32 + 48

String getHtml(char *txt, int len, int type){
    char html[_html_size],s[65],s_ip[16];
    uint32_t ip = WiFi.localIP();
    sprintf(s_ip,"%d.%d.%d.%d",
        ip & 255,
        ip>>8 & 255,
        ip>>16 & 255,
        ip>>24
    );

    snprintf(html,_html_size,"<html>\n<head><title>Wi-Fi コンシェルジェ リモコン担当</title>");
    strcat(html,"<meta http-equiv=\"Content-type\" content=\"text/html; charset=UTF-8\">");
    strcat(html,"</head>");
    strcat(html,"<body>");
    strcat(html,"<h3>IRリモコン STATUS</h3>\n");
    snprintf(html,_html_size,"%s<p>length=%d, data=%s",html,len,txt);
    strcat(html,"</p>\n");
    strcat(html,"<hr>\n");
    strcat(html,"<h3>HTTP GET</h3>");
    snprintf(html,_html_size,"%s<p>http://%s/</p>\n",html,s_ip);
    sprintf(s,"<form method=\"GET\" action=\"http://%s/\">",s_ip);
    strcat(html,s);
    strcat(html,"<input type=\"submit\" value=\"受信データの取得\">");
    strcat(html,"</form>\n");
    strcat(html,"<hr>\n");
    strcat(html,"<h3>HTTP POST</h3>\n");
    sprintf(s,"<form method=\"POST\" action=\"http://%s/\">",s_ip);
    strcat(html,s);
    strcat(html,"<input type=\"text\" name=\"IR\" size=\"30\" value=\"");
    if(len==0) strcat(html,"48,AA,5A,8F,12,16,D1\">");
    else snprintf(html,_html_size,"%s%d,%s\">",html,len,txt);
    strcat(html,"<input type=\"submit\" value=\"リモコン送信(");
    switch(type){
            case AEHA: strcat(html,"AEHA"); break; 
            case NEC : strcat(html,"NEC"); break; 
            case SIRC: strcat(html,"SIRC"); break; 
            default  : strcat(html,"UNKNOWN"); break; 
    }
    strcat(html,")\">");
    strcat(html,"</form>");
    sprintf(s,"<form method=\"GET\" action=\"http://%s/\">",s_ip);
    strcat(html,s);
    strcat(html,"<p>送信時の信号方式の切換え (HTTP GET /?TYPE=方式)</p>");
    strcat(html,"<input type=\"submit\" name=\"TYPE\" value=\"0 (AEHA)\">");
    strcat(html,"<input type=\"submit\" name=\"TYPE\" value=\"1 (NEC)\">");
    strcat(html,"<input type=\"submit\" name=\"TYPE\" value=\"2 (SIRC)\">");
    strcat(html,"</form>");
    strcat(html,"</body>");
    strcat(html,"</html>");
    
    // Serial.println("sizeof(html)=" + String(strlen(html)+1));
    // 21:07:04.422 -> sizeof(html)=874

    return String(html);
}

/*
void html(WiFiClient &client, char *txt, int len, int type, uint32_t ip){
    char s[65],s_ip[16];
    
    sprintf(s_ip,"%d.%d.%d.%d",
        ip & 255,
        ip>>8 & 255,
        ip>>16 & 255,
        ip>>24
    );
    client.println("HTTP/1.1 200 OK");              // HTTP OKを応答
    client.println("Content-Type: text/html");      // HTMLコンテンツ
    client.println("Connection: close");            // 応答終了後にセッションを閉じる
    client.println();
    client.println("<html>");
    client.println("<head><title>IRリモコン</title>");
    client.println("<meta http-equiv=\"Content-type\" content=\"text/html; charset=UTF-8\">");
    client.println("</head>");
    client.println("<body>");
    client.println("<h3>IRリモコン STATUS</h3>");
    client.print("<p>length=");
    client.print(len);
    client.print(", data=");
    client.print(txt);
    client.println("</p>");
    client.println("<hr>");
    client.println("<h3>HTTP GET</h3>");
    client.print("<p>http://");
    client.print(s_ip);
    client.println("/</p>");
    sprintf(s,"<form method=\"GET\" action=\"http://%s/\">",s_ip);
    client.println(s);
    client.println("<input type=\"submit\" value=\"受信データの取得\">");
    client.println("</form>");
    client.println("<hr>");
    client.println("<h3>HTTP POST</h3>");
    sprintf(s,"<form method=\"POST\" action=\"http://%s/\">",s_ip);
    client.println(s);
    client.print("<input type=\"text\" name=\"IR\" size=\"30\" value=\"");
    if(len==0) client.println("48,AA,5A,8F,12,16,D1\">");
    else{
        client.print(len);
        client.print(",");
        client.print(txt);
        client.println("\">");
    }
    client.print("<input type=\"submit\" value=\"リモコン送信(");
    switch(type){
            case AEHA: client.print("AEHA"); break; 
            case NEC : client.print("NEC"); break; 
            case SIRC: client.print("SIRC"); break; 
            default  : client.print("UNKNOWN"); break; 
    }
    client.println(")\">");
    client.println("</form>");
    sprintf(s,"<form method=\"GET\" action=\"http://%s/\">",s_ip);
    client.println(s);
    client.println("<p>送信時の信号方式の切換え (HTTP GET /?TYPE=方式)</p>");
    client.println("<input type=\"submit\" name=\"TYPE\" value=\"0 (AEHA)\">");
    client.println("<input type=\"submit\" name=\"TYPE\" value=\"1 (NEC)\">");
    client.println("<input type=\"submit\" name=\"TYPE\" value=\"2 (SIRC)\">");
    client.println("</form>");
    client.println("</body>");
    client.println("</html>");
}
*/
