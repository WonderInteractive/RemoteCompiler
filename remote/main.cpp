#include <vector>
#include <string>
#include <iostream>
#include "fmt/format.h"
#include <taskflow/taskflow.hpp>
#include "../uSockets/src/libusockets.h"

tf::Executor executor;
tf::Taskflow taskflow;

const int SSL = 0;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct http_socket {
	/* How far we have streamed our response */
	int offset;
};

struct http_context {
	/* The shared response */
	char *response;
	int length;
};

/* We don't need any of these */
void on_wakeup(struct us_loop_t *loop) {

}

void on_pre(struct us_loop_t *loop) {

}

/* This is not HTTP POST, it is merely an event emitted post loop iteration */
void on_post(struct us_loop_t *loop) {

}

struct us_socket_t *on_http_socket_writable(struct us_socket_t *s) {
	struct http_socket *http_socket = (struct http_socket *) us_socket_ext(SSL, s);
	struct http_context *http_context = (struct http_context *) us_socket_context_ext(SSL, us_socket_context(SSL, s));

	/* Stream whatever is remaining of the response */
	http_socket->offset += us_socket_write(SSL, s, http_context->response + http_socket->offset, http_context->length - http_socket->offset, 0);

	return s;
}

struct us_socket_t *on_http_socket_close(struct us_socket_t *s, int code, void *reason) {
	printf("Client disconnected\n");

	return s;
}

struct us_socket_t *on_http_socket_end(struct us_socket_t *s) {
	/* HTTP does not support half-closed sockets */
	us_socket_shutdown(SSL, s);
	return us_socket_close(SSL, s, 0, NULL);
}
//method to read a file
std::string readFile(const std::string& filename) {
	std::ifstream file(filename);
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

//method to write a new file
void writeFile(const std::string& filename, std::string_view content) {
	std::ofstream file(filename);
	file << content;
}



std::string exec(const char* cmd) {
	std::array<char, 128> buffer;
	std::string result;
	std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd, "r"), _pclose);
	if (!pipe) {
		throw std::runtime_error("popen() failed!");
	}
	while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
		result += buffer.data();
	}
	return result;

}
struct us_socket_t *on_http_socket_data(struct us_socket_t *s, char *data, int length) {
	/* Get socket extension and the socket's context's extension */
	struct http_socket *http_socket = (struct http_socket *) us_socket_ext(SSL, s);
	struct http_context *http_context = (struct http_context *) us_socket_context_ext(SSL, us_socket_context(SSL, s));
	auto view = std::string_view(data);
	auto found = view.find("tes");
	auto body_view = view.substr(found);
	auto pos = length - found;
	auto found_newline = body_view.find('|');
	pos -= found_newline;
	auto info_view = body_view.substr(0, found_newline);
	auto main_view = body_view.substr(found_newline + 1, pos - 1);
	std::string cmd = "";
	fmt::print("{} : {} : {} : {}\n", info_view, main_view, found_newline, pos - 1);
	Sleep(10000);
	//call a process
	writeFile("temp.cpp", main_view);

	auto buf2 = exec("\"\"C:/Program Files (x86)/Microsoft Visual Studio\\2019\\BuildTools\\VC\\Tools\\MSVC\\14.29.30133\\bin\\Hostx64\\x86\\cl.exe\" temp.cpp /c \"");

	//read temp.obj
	auto buf3 = readFile("temp.obj");
	//system(cmd.c_str());

	//compress output


	/* We treat all data events as a request */
	auto body = fmt::format("test");
	//auto resp = fmt::format("HTTP/1.1 200 OK\r\nContent-Length: {}\r\n\r\n{}", sizeof(body) - 1, body);
	//http_context->response = resp.data();
	//http_context->length = resp.length();
	//const char body[] = "<html><body><h1>Why hello there!</h1></body></html>";
	http_context->response = (char*)malloc(128 + body.size());
	http_context->length = snprintf(http_context->response, 128 + body.size(), "HTTP/1.1 200 OK\r\nContent-Length: %ld\r\n\r\n%s", body.size(), body.data());
	fmt::print("Recv: {} : {}\n", http_context->response, http_context->length);
	http_socket->offset = us_socket_write(SSL, s, http_context->response, http_context->length, 0);

	/* Reset idle timer */
	us_socket_timeout(SSL, s, 30);

	return s;
}

struct us_socket_t *on_http_socket_open(struct us_socket_t *s, int is_client, char *ip, int ip_length) {
	struct http_socket *http_socket = (struct http_socket *) us_socket_ext(SSL, s);

	/* Reset offset */
	http_socket->offset = 0;

	/* Timeout idle HTTP connections */
	us_socket_timeout(SSL, s, 30);

	printf("Client connected\n");

	return s;
}

struct us_socket_t *on_http_socket_timeout(struct us_socket_t *s) {
	/* Close idle HTTP sockets */
	return us_socket_close(SSL, s, 0, NULL);
}


int main(int argc, char *argv[]) {

    /* Create the event loop */
	struct us_loop_t *loop = us_create_loop(0, on_wakeup, on_pre, on_post, 0);

	struct us_socket_context_options_t options = {};
	options.key_file_name = "../../misc/key.pem";
	options.cert_file_name = "../../misc/cert.pem";
	options.passphrase = "1234";

	struct us_socket_context_t *http_context = us_create_socket_context(SSL, loop, sizeof(struct http_context), options);

	if (!http_context) {
		printf("Could not load SSL cert/key\n");
		exit(0);
	}

	/* Set up event handlers */
	us_socket_context_on_open(SSL, http_context, on_http_socket_open);
	us_socket_context_on_data(SSL, http_context, on_http_socket_data);
	us_socket_context_on_writable(SSL, http_context, on_http_socket_writable);
	us_socket_context_on_close(SSL, http_context, on_http_socket_close);
	us_socket_context_on_timeout(SSL, http_context, on_http_socket_timeout);
	us_socket_context_on_end(SSL, http_context, on_http_socket_end);

	/* Start serving HTTP connections */
	struct us_listen_socket_t *listen_socket = us_socket_context_listen(SSL, http_context, 0, 30000, 0, sizeof(struct http_socket));

	if (listen_socket) {
		printf("Listening on port 30000...\n");
		us_loop_run(loop);
	} else {
		printf("Failed to listen!\n");
	}

    return 0;
}