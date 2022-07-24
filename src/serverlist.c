#include "serverlist.h"
#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>

struct winsize termsize;
char* str_serverlist;
int server_count = 0;
int selected_server = 0;

int sort_by_playercount(json_object* const* j1, json_object* const* j2) {
	int i1 = json_object_get_int(json_object_object_get(*j1, "players_current"));
	int i2 = json_object_get_int(json_object_object_get(*j2, "players_current"));

	return i2 - i1;
}

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
		s.players_current = json_object_get_int(json_object_object_get(json_arr_pos, "players_current"));
		s.players_max = json_object_get_int(json_object_object_get(json_arr_pos, "players_max"));

		servers[i] = s;
	}

	return servers;
}

void draw_serverlist(struct Server* servers) {
	int lines_to_print = termsize.ws_row-2 < server_count ? termsize.ws_row-2 : server_count;
	int servers_left = server_count-selected_server;

	if (servers_left > lines_to_print) {
		for (int i = selected_server; i < server_count; i++)
		{
			if (i > lines_to_print+selected_server) continue;

			if (i == selected_server)
				printf("** (SELECTED) ");

			printf("%s -- %s -- %i/%i\n",
					servers[i].name, servers[i].map,
					servers[i].players_current, servers[i].players_max);
		}
	} else {
		for (int i = lines_to_print-7; i < server_count; i++)
		{
			if (i == selected_server)
				printf("** (SELECTED) ");

			printf("%s -- %s -- %i/%i\n",
					servers[i].name, servers[i].map,
					servers[i].players_current, servers[i].players_max);
		}
	}
}

void serverlist_scene_init() {
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &termsize);

	get_serverlist();

	json_object* root = json_tokener_parse(str_serverlist);
	json_object_array_sort(root, sort_by_playercount);
	struct Server* servers = get_servers_array(root);

	serverlist_loop(servers);

	free(servers);
}

void serverlist_loop(struct Server* servers) {
	int disable_handler = 0;
	while(1) {
		int option;
		printf("\033[H\033[J");
		draw_serverlist(servers);

		tcflush(STDIN_FILENO, TCIFLUSH);
		while(read(STDIN_FILENO, &option, 1) < 1);

		// escape code
		if (option == 27){
			while(read(STDIN_FILENO, &option, 1) < 1);
			if (option == 91)
				while(read(STDIN_FILENO, &option, 1) < 1);
		}

		// up
		if (option == 65) {
			selected_server--;
		// down
		} else if (option == 66) {
			selected_server++;
		// enter
		} else if (option == 10) {
			join_to_server(servers[selected_server]);
		}

		if (selected_server < 0){
			selected_server = server_count;
		} else if (selected_server > server_count) {
			selected_server = 0;
		}

		if (option == 'c') break;
	}
}

void join_to_server(struct Server selected_sv) {
	char cmd[500];
	sprintf(cmd, "WINEPREFIX=~/aos_wine wine \"$HOME/aos_wine/drive_c/Ace of Spades/client.exe\" -%s", selected_sv.identifier);
	int ret = system(cmd);

	if (WEXITSTATUS(ret) != 0) {
		exit(-1);
	}
}