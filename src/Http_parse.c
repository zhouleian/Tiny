#include "Http.h"
#include "Http_parse.h"
#include "DefineConf.h"
#include "Log.h"
/*
int http_parse_request_line(http_request_t *r) {
	int method;
	u_char *s;
	size_t pi;
	s = (u_char*)&r->buf;
	char ch = *s;
	while(ch == CR || ch == LF)
		ch++;
	if ((ch < 'A' || ch > 'Z') && ch != '_') {
		return HTTP_PARSE_INVALID_METHOD;//首先判断是合法的方法
	}
	char d[MAX_BUF];
	//下面解析方法method,get,post,head
	strncpy(d,s,3); 
	if(strcasecmp(d,"GET")){
		strncpy(d,s,4); 
		if(strcasecmp(d,"POST")){
			strncpy(d,s,4); 
			if(strcasecmp(d,"HEAD")){
				r->method = HTTP_UNKNOWN;
			}else{
				r->method = HTTP_HEAD;
				s +=5;
				pi+=5;
				ch =*s;
			}
		}else{
			r->method = HTTP_POST;
			s+=5;
			pi+=5;
			ch =*s;
		}
	}else{
		r->method = HTTP_GET;
		s += 4;
		pi+=4;
		ch = *s;
	}
	//下面解析uri
	r->uri_start = s;
	while(ch !=' '){
		s++;
		pi++;
		ch=*s;
	}
	r->uri_end = s;

	//HTTP解析
	s++;pi++;
	char http_str[BUFSIZE];
	strncpy(http_str,s,5);
	if(strcasecmp(http_str,"HTTP/")){
		return HTTP_PARSE_INVALID_REQUEST;
	}else{
		s+=5;
		pi+=5;
	}

	//解析http的数字版本
	ch = *s;
	if (ch < '1' || ch > '9') {
		return HTTP_PARSE_INVALID_REQUEST;
	}else{
		s++;
		pi++;
	}
	r->http_major = ch-'0';
	ch = *s;
	//FIXME:需要修正，现在还只适合于单数字的情况
	if(ch =='.'){//http_major结束
		s++;
		pi++;
		ch = *s;
		if(ch<'1'||ch>'9'){//这需要至少从1.1开始才可以，1.0会返回invalid_request
			return HTTP_PARSE_INVALID_REQUEST;
		}else 
			r->http_minor = ch-'0';
	}else if(ch <'1'||ch>'9'){
		return HTTP_PARSE_INVALID_REQUEST;
	}else{
		s++;
		pi++;
		r->http_major = r->http_major*10 + ch-'0';
	}


	//下面是解析一些其他的参数
}
*/
//还是用switch方法更好，因为uri里面涉及到解析时候的循环，不用switch不好写
int http_parse_request_line(http_request_t *r) {
    u_char ch, *p, *m;
    size_t pi;

    enum {
        sw_start = 0,
        sw_method,
        sw_spaces_before_uri,
        sw_after_slash_in_uri,
        sw_http,
        sw_http_H,
        sw_http_HT,
        sw_http_HTT,
        sw_http_HTTP,
        sw_first_major_digit,
        sw_major_digit,
        sw_first_minor_digit,
        sw_minor_digit,
        sw_spaces_after_digit,
        sw_almost_done
    } state;

    state = r->state;

    for (pi = r->pos; pi < r->last; pi++) {//last是已经读取客户请求之后的总长度
        p = (u_char *)&r->buf[pi % MAX_BUF];
        ch = *p;

        switch (state) {

        /* http方法：get,head,post*/
        case sw_start:
            r->request_start = p;

            if (ch == CR || ch == LF) {
                break;
            }

            if ((ch < 'A' || ch > 'Z') && ch != '_') {
                return HTTP_PARSE_INVALID_METHOD;
            }

            state = sw_method;
            break;

        case sw_method:
            if (ch == ' ') {
                r->method_end = p;
                m = r->request_start;

                switch (p - m) {

                case 3:
                    if (str3_cmp(m, 'G', 'E', 'T', ' ')) {
                        r->method = HTTP_GET;
                        break;
                    }

                    break;

                case 4:
                    if (str3Ocmp(m, 'P', 'O', 'S', 'T')) {
                        r->method = HTTP_POST;
                        break;
                    }

                    if (str4cmp(m, 'H', 'E', 'A', 'D')) {
                        r->method = HTTP_HEAD;
                        break;
                    }

                    break;
                default:
                    r->method = HTTP_UNKNOWN;
                    break;
                }
                state = sw_spaces_before_uri;
                break;
            }

            if ((ch < 'A' || ch > 'Z') && ch != '_') {
                return HTTP_PARSE_INVALID_METHOD;
            }

            break;

        /* space* before URI */
        case sw_spaces_before_uri:

            if (ch == '/') {
                r->uri_start = p;
                state = sw_after_slash_in_uri;
                break;
            }

            switch (ch) {
                case ' ':
                    break;
                default:
                    return HTTP_PARSE_INVALID_REQUEST;
            }
            break;

        case sw_after_slash_in_uri:

            switch (ch) {
            case ' ':
                r->uri_end = p;
                state = sw_http;
                break;
            default:
                break;
            }
            break;

        /* space+ after URI */
        case sw_http:
            switch (ch) {
            case ' ':
                break;
            case 'H':
                state = sw_http_H;
                break;
            default:
                return HTTP_PARSE_INVALID_REQUEST;
            }
            break;

        case sw_http_H:
            switch (ch) {
            case 'T':
                state = sw_http_HT;
                break;
            default:
                return HTTP_PARSE_INVALID_REQUEST;
            }
            break;

        case sw_http_HT:
            switch (ch) {
            case 'T':
                state = sw_http_HTT;
                break;
            default:
                return HTTP_PARSE_INVALID_REQUEST;
            }
            break;

        case sw_http_HTT:
            switch (ch) {
            case 'P':
                state = sw_http_HTTP;
                break;
            default:
                return HTTP_PARSE_INVALID_REQUEST;
            }
            break;

        case sw_http_HTTP:
            switch (ch) {
            case '/':
                state = sw_first_major_digit;
                break;
            default:
                return HTTP_PARSE_INVALID_REQUEST;
            }
            break;

        /* first digit of major HTTP version */
        case sw_first_major_digit:
            if (ch < '1' || ch > '9') {
                return HTTP_PARSE_INVALID_REQUEST;
            }

            r->http_major = ch - '0';
            state = sw_major_digit;
            break;

        /* major HTTP version or dot */
        case sw_major_digit:
            if (ch == '.') {
                state = sw_first_minor_digit;
                break;
            }

            if (ch < '0' || ch > '9') {
                return HTTP_PARSE_INVALID_REQUEST;
            }

            r->http_major = r->http_major * 10 + ch - '0';
            break;
//HTTP版本
        case sw_first_minor_digit:
            if (ch < '0' || ch > '9') {
                return HTTP_PARSE_INVALID_REQUEST;
            }

            r->http_minor = ch - '0';
            state = sw_minor_digit;
            break;

        case sw_minor_digit:
            if (ch == CR) {
                state = sw_almost_done;
                break;
            }

            if (ch == LF) {
                goto done;
            }

            if (ch == ' ') {
                state = sw_spaces_after_digit;
                break;
            }

            if (ch < '0' || ch > '9') {
                return HTTP_PARSE_INVALID_REQUEST;
            }

            r->http_minor = r->http_minor * 10 + ch - '0';
            break;

        case sw_spaces_after_digit:
            switch (ch) {
            case ' ':
                break;
            case CR:
                state = sw_almost_done;
                break;
            case LF:
                goto done;
            default:
                return HTTP_PARSE_INVALID_REQUEST;
            }
            break;

        //请求行结束之后是一个空行，表示结束
        case sw_almost_done:
            r->request_end = p - 1;
            switch (ch) {
            case LF:
                goto done;
            default:
                return HTTP_PARSE_INVALID_REQUEST;
            }
        }
    }

    r->pos = pi;
    r->state = state;

    return EAGAIN;

done:

    r->pos = pi + 1;

    if (r->request_end == NULL) {
        r->request_end = p;
    }

    r->state = sw_start;

    return OK;
}


int http_parse_request_body(http_request_t *r) {
	u_char ch, *p;
	size_t pi;

	enum {
		sw_start = 0,
		sw_key,
		sw_spaces_before_colon,
		sw_spaces_after_colon,
		sw_value,
		sw_cr,
		sw_crlf,
		sw_crlfcr
	} state;

	state = r->state;
	check(state == 0, "state should be 0");


	http_header_t *hd; 
	for (pi = r->pos; pi < r->last; pi++) {
		p = (u_char *)&r->buf[pi % MAX_BUF];
		ch = *p;

		switch (state) {
			case sw_start:
				if (ch == CR || ch == LF) {
					break;
				}

				r->cur_header_key_start = p;
				state = sw_key;
				break;
			case sw_key:
				if (ch == ' ') {
					r->cur_header_key_end = p;
					state = sw_spaces_before_colon;
					break;
				}

				if (ch == ':') {
					r->cur_header_key_end = p;
					state = sw_spaces_after_colon;
					break;
				}

				break;
			case sw_spaces_before_colon:
				if (ch == ' ') {
					break;
				} else if (ch == ':') {
					state = sw_spaces_after_colon;
					break;
				} else {
					return HTTP_PARSE_INVALID_HEADER;
				}
			case sw_spaces_after_colon:
				if (ch == ' ') {
					break;
				}

				state = sw_value;
				r->cur_header_value_start = p;
				break;
			case sw_value:
				if (ch == CR) {
					r->cur_header_value_end = p;
					state = sw_cr;
				}

				if (ch == LF) {
					r->cur_header_value_end = p;
					state = sw_crlf;
				}

				break;
			case sw_cr:
				if (ch == LF) {
					state = sw_crlf;
					hd = (http_header_t *)malloc(sizeof(http_header_t));
					hd->key_start   = r->cur_header_key_start;
					hd->key_end     = r->cur_header_key_end;
					hd->value_start = r->cur_header_value_start;
					hd->value_end   = r->cur_header_value_end;

					list_add(&(hd->list), &(r->list));

					break;
				} else {
					return HTTP_PARSE_INVALID_HEADER;
				}

			case sw_crlf:
				if (ch == CR) {
					state = sw_crlfcr;
				} else {
					r->cur_header_key_start = p;
					state = sw_key;
				}
				break;

			case sw_crlfcr:
				switch (ch) {
					case LF:
						goto done;
					default:
						return HTTP_PARSE_INVALID_HEADER;
				}
				break;
		}   
	}

	r->pos = pi;
	r->state = state;

	return EAGAIN;

done:
	r->pos = pi + 1;

	r->state = sw_start;

	return OK;
}

