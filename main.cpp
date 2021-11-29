/* 
Referenced in https://gamehacking.academy/lesson/4/4.

A hack for Flare version 1.12 that will automatically move the player towards the nearest enemy and then attack until the enemy is dead.

It accomplishes this through a thread that constantly compares the current player's X and Y position to an enemy's position. 
Depending on the position, the mouse cursor is then set to a certain angle and SendInput is used to send a mouse down event to the game.

After injecting this hack, go in game and hold down the "M" key. Your character will then begin running toward the closest enemy and begin attacking them.

This must be injected into the Flare process to work. One way to do this is to use a DLL injector. Another way is to enable AppInit_DLLs in the registry.
*/
#include <Windows.h>

HANDLE flare_base;

DWORD* mouse_x = NULL;
DWORD* mouse_y = NULL;

DWORD mouse_call_address;
DWORD mouse_return_address;

float* player_x = NULL;
float* player_y = NULL;

DWORD player_call_address;
DWORD player_return_address;

float* enemy_x = NULL;
float* enemy_y = NULL;

DWORD loop_call_address;
DWORD loop_return_address;

INPUT input = { 0 };

// This hooks the method responsible for updating the mouse cursor
// The main job of this code cave is retrieve the mouse's position addresses and assign them to a pointer
__declspec(naked) void mouse_codecave() {
	__asm {
		call mouse_call_address
		pushad
		mov eax, ebp
		add eax, 0x664
		mov ebx, ebp
		add ebx, 0x668
		mov mouse_x, eax
		mov mouse_y, ebx
		popad
		jmp mouse_return_address
	}
}

// This hooks the method responsible for updating the player's position
// The main job of this code cave is retrieve the player's position addresses and assign them to a pointer
__declspec(naked) void player_codecave() {
	__asm {
		pushad
		mov eax, ecx 
		add eax, 0x240
		mov player_x, eax 
		add eax, 4
		mov player_y, eax
		popad
		call player_call_address
		jmp player_return_address
	}
}

// This hooks the method responsible for updating the enemy's position
// The main job of this code cave is retrieve the enemy's position addresses and assign them to a pointer
__declspec(naked) void loop_codecave() {
	__asm {
		pushad
		mov eax, ebx
		sub eax, 4
		mov enemy_x, eax
		mov enemy_y, ebx
		popad
		call loop_call_address
		jmp loop_return_address
	}
}

// Main logic thread of the bot
// Checks to ensure that all the pointers are assigned
// If they are, that means an enemy is on screen. As such, we determine the position of the enemy
// and send our player toward that position
void injected_thread() {
	while (true) {
		if (player_x != NULL && player_y != NULL && enemy_x != NULL && enemy_y != NULL 
			&& player_x != enemy_x && player_y != enemy_y
			&& mouse_x != NULL && mouse_y != NULL && GetAsyncKeyState('M')) {

			if (*enemy_x < *player_x) {
				*mouse_x = 490;
			}
			else {
				*mouse_x = 560;
			}

			if (*enemy_y < *player_y) {
				*mouse_y = 270;
			}
			else {
				*mouse_y = 330;
			}

			input.type = INPUT_MOUSE;
			input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
			SendInput(1, &input, sizeof(INPUT));
		}

		Sleep(1);
	}
}

// When our DLL is attached, we create several code caves and a thread
// For each hook, unprotect the memory at the code we wish to write at
// Then set the first opcode to E9, or jump
// Caculate the location using the formula: new_location - original_location+5
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	DWORD old_protect;

	if (fdwReason == DLL_PROCESS_ATTACH) {
		flare_base = GetModuleHandle(L"flare.exe");

		mouse_call_address = (DWORD)flare_base + 0x54210;

		unsigned char* hook_location = (unsigned char*)((DWORD)flare_base + 0xECBC8);
		mouse_return_address = (DWORD)hook_location + 5;

		VirtualProtect((void*)hook_location, 5, PAGE_EXECUTE_READWRITE, &old_protect);
		*hook_location = 0xE9;
		*(DWORD*)(hook_location + 1) = (DWORD)&mouse_codecave - ((DWORD)hook_location + 5);

		player_call_address = (DWORD)flare_base + 0x20840;

		hook_location = (unsigned char*)((DWORD)flare_base + 0xCAC4);
		player_return_address = (DWORD)hook_location + 5;

		VirtualProtect((void*)hook_location, 5, PAGE_EXECUTE_READWRITE, &old_protect);
		*hook_location = 0xE9;
		*(DWORD*)(hook_location + 1) = (DWORD)&player_codecave - ((DWORD)hook_location + 5);

		loop_call_address = (DWORD)flare_base + 0x6B180;

		hook_location = (unsigned char*)((DWORD)flare_base + 0x6BA94);
		loop_return_address = (DWORD)hook_location + 5;

		VirtualProtect((void*)hook_location, 5, PAGE_EXECUTE_READWRITE, &old_protect);
		*hook_location = 0xE9;
		*(DWORD*)(hook_location + 1) = (DWORD)&loop_codecave - ((DWORD)hook_location + 5);

		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)injected_thread, NULL, 0, NULL);
	}

	return true;
}
