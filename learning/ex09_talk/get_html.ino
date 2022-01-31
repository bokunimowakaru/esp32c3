/*******************************************************************************
HTMLコンテンツ AquesTalk

                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/

#define _html_size 1369     // 1239 + 32 + 98

String getHtml(char *talk){
    char html[_html_size],s[65],s_ip[16];
    uint32_t ip = WiFi.localIP();

    sprintf(s_ip,"%d.%d.%d.%d",
        ip & 255,
        ip>>8 & 255,
        ip>>16 & 255,
        ip>>24
    );

    snprintf(html,_html_size,"<html>\n<head><title>Wi-Fi コンシェルジェ 音声アナウンス担当</title>\n");
    strcat(html,"<meta http-equiv=\"Content-type\" content=\"text/html; charset=UTF-8\">\n");
    strcat(html,"</head>\n");
    strcat(html,"<body>\n");
    strcat(html,"<h3>AquesTalk STATUS</h3>\n");
    strcat(html,"<p>");
    for(int i=0; i< strlen(talk); i++){
        if(talk[i] == '<') strcat(html,"&lt;");
        else if(talk[i] == '>') strcat(html,"&gt;");
        else{
            int j = strlen(html);
            html[j] = talk[i];
            html[j+1] = '\0';
        }
    }
    strcat(html,"</p>\n");
    strcat(html,"<hr>\n");
    strcat(html,"<h3>HTTP GET</h3>\n");
    strcat(html,"<p>http://");
    strcat(html,s_ip);
    strcat(html,"/?TEXT=文字列</p>\n");
    sprintf(s,"<form method=\"GET\" action=\"http://%s/\">",s_ip);
    strcat(html,s);
    strcat(html,"<input type=\"submit\" name=\"TEXT\" value=\"oshaberiaio-thi-ta'nnma_tsu.\">\n");
    strcat(html,"<input type=\"submit\" name=\"TEXT\" value=\"ku'nino/wataru.\">\n");
    strcat(html,"<input type=\"submit\" name=\"TEXT\" value=\"#J\">\n");
    strcat(html,"<input type=\"submit\" name=\"TEXT\" value=\"#K\">\n");
    sprintf(html,"%s<input type=\"submit\" name=\"VAL\" value=\"%d\">\n",html,(int)(ip>>24));
    strcat(html,"</form>\n");
    strcat(html,s);
    strcat(html,"<input type=\"text\" name=\"TEXT\" value=\"\">\n");
    strcat(html,"<input type=\"submit\" value=\"送信\">\n");
    strcat(html,"</form>\n");
    strcat(html,"<hr>\n");
    strcat(html,"<h3>HTTP POST</h3>\n");
    sprintf(s,"<form method=\"POST\" action=\"http://%s/\">",s_ip);
    strcat(html,s);
    strcat(html,"<input type=\"submit\" name=\"TEXT\" value=\"oshaberiaio-thi-ta'nnma_tsu.\">\n");
    strcat(html,"<input type=\"submit\" name=\"TEXT\" value=\"ku'nino/wataru.\">\n");
    strcat(html,"<input type=\"submit\" name=\"TEXT\" value=\"#J\">\n");
    strcat(html,"<input type=\"submit\" name=\"TEXT\" value=\"#K\">\n");
    sprintf(html,"%s<input type=\"submit\" name=\"VAL\" value=\"%d\">\n",html,(int)(ip>>24));
    strcat(html,"</form>\n");
    strcat(html,s);
    strcat(html,"<input type=\"text\" name=\"TEXT\" value=\"\">\n");
    strcat(html,"<input type=\"submit\" value=\"送信\">\n");
    strcat(html,"</form>\n");
    strcat(html,"</body>\n");
    strcat(html,"</html>\n");

    // Serial.print("<NUM VAL=" + String(strlen(html)+1) + ">.\r");
    return String(html);
}
