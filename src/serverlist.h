#include <stdio.h>
#include <curl/curl.h>
#include <string.h>
#include <json-c/json.h>

struct Server {
	char name[64];
	char identifier[64];
	char map[32];
	char game_mode[16];
	char country[3];
};

void draw_serverlist();
void serverlist_scene_init();
void get_serverlist();
void get_data();
void serverlist_loop();
