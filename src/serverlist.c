#include "serverlist.h"
#include <unistd.h>
#include <sys/ioctl.h>

struct winsize termsize;
char* str_serverlist;
int server_count = 0;
int selected_server = 0;

void get_data(void* ptr, size_t size, size_t nmemb, void* userdata) {
	str_serverlist = ptr;
}

void get_serverlist() {
	CURL* curl;
	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, "https://services.buildandshoot.com/serverlist.json");
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, get_data);

	curl_easy_perform(curl);
}

struct Server* get_servers_array(json_object* array_servers) {
	server_count = json_object_array_length(array_servers);
	struct Server* servers = malloc(server_count*sizeof(*servers));

	for (int i = 0; i < server_count; i++) {
		json_object* json_arr_pos = json_object_array_get_idx(array_servers, i);
		struct Server s;
		strncpy(s.name, json_object_get_string(json_object_object_get(json_arr_pos, "name")), 64);
		strncpy(s.identifier, json_object_get_string(json_object_object_get(json_arr_pos, "identifier")), 64);
		strncpy(s.map, json_object_get_string(json_object_object_get(json_arr_pos, "map")), 32);
		strncpy(s.game_mode, json_object_get_string(json_object_object_get(json_arr_pos, "game_mode")), 16);
		strncpy(s.country, json_object_get_string(json_object_object_get(json_arr_pos, "country")), 3);

		servers[i] = s;
	}

	return servers;
}

void draw_serverlist(struct Server* servers) {
	int lines_to_print = termsize.ws_row-2 < server_count ? termsize.ws_row-2 : server_count;
	for (int i = selected_server; i < server_count; i++)
	{
		if (i > lines_to_print+selected_server) continue;

		if (i == selected_server)
			printf("** (SELECTED) ");

		printf("%s -- %s\n", servers[i].name, servers[i].map);
	}
}

void serverlist_scene_init() {
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &termsize);

	get_serverlist();

	json_object* root = json_tokener_parse(str_serverlist);
	struct Server* servers = get_servers_array(root);

	serverlist_loop(servers);

	free(servers);
}

void serverlist_loop(struct Server* servers) {
	char option;
	while(option != 'c') {
		printf("\033[H\033[J");
		draw_serverlist(servers);
		while(read(STDIN_FILENO, &option, 1) < 1);

		if (option == 66) {
			selected_server++;
		} else if (option == 65) {
			selected_server--;
		}

		if (selected_server < 0){
			selected_server = server_count;
		} else if (selected_server > server_count) {
			selected_server = 0;
		}
	}
}