# AssaultCube 1.3.0.2 x86: offsets and memory layout

A build-specific reference for the local player, other player entities, camera, rendering matrices, position, head position, all weapon ammunition counters, and selected local-server structures.

This map contains **31 globals and a 108-field client player layout**, together with weapon objects, weapon configuration fields, and selected server-client fields. It was derived from the actual Windows executable, checked against the matching official source tag, and sampled using read-only process-memory access. It covers the documented structures, not every function and data item in the program.

## Exact build

| Property | Value |
| --- | --- |
| Executable | ac_client.exe |
| Release | AssaultCube 1.3.0.2, Windows x86 |
| SHA-256 | 5b588e6ff7710fa0170553506a50bfa14750ccb438a154e4ea0e0b1ba621cefb |
| Preferred image base | 0x00400000 |
| Official source tag | v1.3.0.2 |
| Source commit | 1ece5af7533983bb8f827616381d282470793d90 |
| Analysis date | 2026-10-19 |

Match the executable hash before relying on these offsets. A version label alone does not guarantee the same binary or compiler layout. All pointers in this target are **32-bit**, even when the reader runs on 64-bit Windows.

## Files

- [Machine-readable offsets](offsets/offsets.json): globals, RVAs, field offsets, weapon IDs, and evidence. JSON numbers use decimal notation; their values match the hexadecimal values in this guide.
- [C++17 constants](offsets/offsets.hpp): module RVAs and structure offsets.
- [IDA structure declarations](offsets/ida_types.h): flat, fixed-width player and weapon layouts.
- [IDA annotation script](offsets/annotate_ida.py): imports types and adds names/comments after checking the executable hash.
- [Cheat Engine table](offsets/AssaultCube-1.3.0.2.CT): player fields, all weapon counters, camera, globals, and matrices. It contains no Auto Assembler scripts or automatic freezes.
- [Disassembly evidence](offsets/evidence-disassembly.txt): instructions supporting the key identifications.
- [Validation summary](validation/summary.json): checks and their limitations, without machine-specific paths or process snapshots.

## VA, RVA, and field offset

- **VA:** an address in a particular loaded image, such as 0x0058AC00 when the module is at 0x00400000.
- **RVA:** displacement from the image base. Here, 0x0058AC00 - 0x00400000 = 0x18AC00.
- **Field offset:** displacement from an object pointer. Health is at player + 0xEC.

Throughout this guide:

    M = current base address of ac_client.exe
    read_u32(a) = read an unsigned 32-bit integer/pointer at address a
    read_i32(a) = read a signed 32-bit integer at address a
    read_f32(a) = read a 32-bit float at address a

IDA uses suffix notation for hexadecimal numbers: **444h equals 0x444**, or 1092 decimal. It is not decimal 444.

### What survives a restart?

The **RVA and structure offsets remain applicable to the same executable**. Heap pointers must be read again. The module base may also vary, so resolve it instead of assuming a fixed address.

    M + 0x18AC00       address of the global player1 pointer
    P = read_u32(...)  current heap address of the local player
    P + 0x140         address of the assault-rifle magazine counter

The pointer value P can change even when M stays at 0x00400000. Keep the pointer path, not a heap address copied from one session. The local-player pointer changed during the observed process restart, while this module-relative path remained usable.

## Quick reference

    P = read_u32(M + 0x18AC00)                  local playerent*
    C = read_u32(M + 0x17E0A8)                  active camera physent*
    L = read_u32(M + 0x18AC04)                  other-player pointer buffer
    N = read_i32(M + 0x18AC0C)                  buffer length, including null slots
    E_i = read_u32(L + i*4), 0 <= i < N         entity pointer at slot i

    P + 0x004                                  origin/position, three floats
    P + 0x010                                  velocity, three floats
    P + 0x034                                  yaw, float degrees
    P + 0x038                                  pitch, float degrees
    P + 0x03C                                  roll, float degrees
    P + 0x050                                  eyeheight, float
    P + 0x076                                  life state, one byte
    P + 0x077                                  entity type, one byte
    P + 0x0EC                                  health, int32
    P + 0x0F0                                  armour, int32
    P + 0x0FC                                  selected weapon ID, int32
    P + 0x104 + weapon_id*4                     reserve ammo, int32
    P + 0x128 + weapon_id*4                     magazine count, int32
    P + 0x14C + weapon_id*4                     weapon wait/cooldown, int32
    P + 0x1C4                                  client number, int32
    P + 0x205                                  name, inline char[260]
    P + 0x30C                                  team, int32
    read_u32(P + 0x364)                         selected weapon object
    P + 0x3F8                                  head position, three floats

    M + 0x17E010                               modelview matrix, float[16]
    M + 0x17E0B0                               projection matrix, float[16]
    M + 0x17DFD0                               projection * modelview, float[16]

Examples:

    health = read_i32(read_u32(M + 0x18AC00) + 0xEC)
    assault_magazine = read_i32(read_u32(M + 0x18AC00) + 0x140)
    assault_reserve = read_i32(read_u32(M + 0x18AC00) + 0x11C)

There is a dereference between the global RVA and the field offset. Adding M + 0x18AC00 + 0x140 skips that dereference and points somewhere else.

## Client state versus the integrated local server

Two addresses showing the same ammunition value are not sufficient evidence that one is a graphical copy. The process includes client-side state and an integrated server, with different object layouts.

The original ammunition investigation found:

    00494EFD  mov edx, [esi+ecx*4+444h]
    ; ...
    00494F27  lea eax, [edx-1]
    00494F2A  mov [esi+ecx*4+444h], eax

The byte sequence for the write is:

    89 84 8E 44 04 00 00

It is present at VA 0x00494F2A, RVA 0x94F2A. The containing routine at 0x00494EA0 matches the server's processevent(client *c, shotevent &e).

Here, **ESI is a server client pointer**, and ECX is the weapon ID. The server magazine array starts at client + 0x444. For assault rifle ID 6:

    server magazine offset = 0x444 + 6*4 = 0x45C
    client magazine offset = 0x128 + 6*4 = 0x140

Both can describe ammunition consumed by gameplay. They are fields in different structures. The second originally scanned address was not supplied, so its identity has not been established.

To reach local-server objects:

    S = read_u32(M + 0x18B0B8)
    SN = read_i32(M + 0x18B0C0)
    SC_i = read_u32(S + i*4), 0 <= i < SN
    server_magazine = read_i32(SC_i + 0x444 + weapon_id*4)

Server slots whose type is zero are empty. Match client and server objects using clientnum; do not assume identical pointers or vector slots. These objects describe the server hosted locally by this process. An external server's authoritative state resides in that server's process.

## Entity-list traversal

The players collection uses Cube's custom vector, not std::vector:

    struct Vector32 {
        uint32_t buffer;    // +0x00
        int32_t capacity;   // +0x04
        int32_t length;     // +0x08
    };

The vector starts at M + 0x18AC04. Its buffer holds four-byte player pointers. The local player is handled separately; this vector describes other clients and can contain null slots. Its length is not the number of living enemies.

Validate the vector bounds, then check each pointer, life state, and team. The disassembly at 0x00479723 and 0x00479740 confirms the length check and indexed pointer access. The runtime sample had no other clients, so traversal was not dynamically exercised with bots.

## Position, camera, and head position

### Position

The o vector at +0x04 is the physics/view origin, not an animated head bone or foot position. X and Y are horizontal; Z is vertical. A foot-position estimate is:

    feet = (o.x, o.y, o.z - eyeheight)

Interpolation, crouching, and animation affect interpretation. The source's resetinterp() derives newpos.z by subtracting eye height from o.z.

### Camera

The pointer read from M + 0x17E0A8 yields the active physent. It normally equals the local player. Death cameras, spectator modes, and rendering passes can select another object. Read its physics fields; do not assume every camera object has a complete playerent layout.

Yaw, pitch, and roll are at +0x34, +0x38, and +0x3C. Camera position is its own o vector at +0x04.

### Head

The vector at playerent + 0x3F8 stores the world-space head position. The renderer obtains it from the model attachment tag_head for other living players during the appropriate pass.

The local player is excluded from this update. The initial/invalid value is (-1, -1, -1). A head vector can be stale when its model has not been rendered; it is not a continuously valid bone query. The field and initialization have constructor/source evidence, but runtime animation behavior was not tested with a bot in the sample.

## Matrices and world-to-screen projection

Matrices contain **16 floats in OpenGL column-major order**:

    modelview      M + 0x17E010
    projection     M + 0x17E0B0
    MVP            M + 0x17DFD0
    inverse view   M + 0x17E060
    inverse MVP    M + 0x17DF90

The routine readmatrices() at 0x004570D0 calls glGetFloatv with GL_MODELVIEW_MATRIX (0xBA6) and GL_PROJECTION_MATRIX (0xBA7), then computes MVP = projection * modelview and the inverse matrices.

Given world-space point (x,y,z) and MVP array m:

    clip_x = x*m[0] + y*m[4] + z*m[8]  + m[12]
    clip_y = x*m[1] + y*m[5] + z*m[9]  + m[13]
    clip_z = x*m[2] + y*m[6] + z*m[10] + m[14]
    clip_w = x*m[3] + y*m[7] + z*m[11] + m[15]

Reject points with clip_w <= epsilon. Otherwise:

    ndc_x = clip_x / clip_w
    ndc_y = clip_y / clip_w
    ndc_z = clip_z / clip_w
    screen_x = (ndc_x + 1) * screen_width / 2
    screen_y = (1 - ndc_y) * screen_height / 2

For frustum inclusion, check all three normalized coordinates against [-1,1]. This does not test wall occlusion. The formulas assume a main viewport filling the window and a top-left screen origin. Width and height are at M+0x191ED8 and M+0x191EDC.

External reads can occur while rendering updates the matrices or camera. The validation sample was not frame-synchronized.

## All weapon IDs and counters

The ammo array stores reserve ammunition; mag stores magazine content; gunwait stores each weapon's wait counter. Each array has nine four-byte elements. Knife and grenade behavior differs from conventional magazine-fed firearms. A slot does not establish that its weapon is owned or selectable.

| ID | Weapon | Reserve offset | Magazine offset | Wait offset | Weapon pointer offset | Server reserve | Server magazine |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 0 | knife | 0x104 | 0x128 | 0x14C | 0x33C | 0x420 | 0x444 |
| 1 | pistol | 0x108 | 0x12C | 0x150 | 0x340 | 0x424 | 0x448 |
| 2 | carbine | 0x10C | 0x130 | 0x154 | 0x344 | 0x428 | 0x44C |
| 3 | shotgun | 0x110 | 0x134 | 0x158 | 0x348 | 0x42C | 0x450 |
| 4 | subgun | 0x114 | 0x138 | 0x15C | 0x34C | 0x430 | 0x454 |
| 5 | sniper | 0x118 | 0x13C | 0x160 | 0x350 | 0x434 | 0x458 |
| 6 | assault | 0x11C | 0x140 | 0x164 | 0x354 | 0x438 | 0x45C |
| 7 | grenade | 0x120 | 0x144 | 0x168 | 0x358 | 0x43C | 0x460 |
| 8 | akimbo | 0x124 | 0x148 | 0x16C | 0x35C | 0x440 | 0x464 |

## Globals

VA values assume image base 0x00400000. Resolve runtime_module_base + RVA, or IDA_image_base + RVA for a rebased database. Pointer globals require a dereference; matrix arrays are stored directly at their addresses.

| Name | VA | RVA | Type | Evidence |
| --- | --- | --- | --- | --- |
| player1 | 0x0058AC00 | 0x18AC00 | playerent* | 0x4090E0 initializes with newplayerent() |
| players_buf | 0x0058AC04 | 0x18AC04 | playerent** | 0x479740 indexes buf[i] and checks null |
| players_capacity | 0x0058AC08 | 0x18AC08 | int | Cube vector<T>: buf, alen, ulen |
| players_length | 0x0058AC0C | 0x18AC0C | int | 0x479723 / 0x479790 loop bounds |
| camera1 | 0x0057E0A8 | 0x17E0A8 | physent* | 0x457498 and 0x479734; normally player1 |
| mvmatrix | 0x0057E010 | 0x17E010 | float[16] | 0x4570D9; GL_MODELVIEW_MATRIX=0xBA6 |
| projmatrix | 0x0057E0B0 | 0x17E0B0 | float[16] | 0x4570E9; GL_PROJECTION_MATRIX=0xBA7 |
| mvpmatrix | 0x0057DFD0 | 0x17DFD0 | float[16] | 0x457101 destination; projection * modelview |
| invmvmatrix | 0x0057E060 | 0x17E060 | float[16] | 0x4571AE / 0x4571B3 |
| invmvpmatrix | 0x0057DF90 | 0x17DF90 | float[16] | 0x457280 / 0x457285 |
| camright | 0x00592088 | 0x192088 | vec3 | readmatrices: mvmatrix[0,4,8] |
| camup | 0x0059207C | 0x19207C | vec3 | readmatrices: mvmatrix[1,5,9] |
| camdir | 0x00592070 | 0x192070 | vec3 | readmatrices: -mvmatrix[2,6,10] |
| aspect | 0x0057E0A0 | 0x17E0A0 | float | 0x457570 perspective setup; projection[5]/projection[0] |
| fovy | 0x0057E0A4 | 0x17E0A4 | float | 0x4574D7 perspective setup; 2*atan(1/projection[5]) |
| farplane | 0x0057E05C | 0x17E05C | int | 0x4574C7 perspective setup |
| fov | 0x0058A7CC | 0x18A7CC | float | 0x406D5D / 0x406D6A FVAR registration |
| scopefov | 0x0058A834 | 0x18A834 | int | 0x406D87 registration |
| spectfov | 0x0058A838 | 0x18A838 | int | 0x406DB4 registration |
| spectfovremote | 0x0058A83C | 0x18A83C | int | 0x406DE4 registration |
| screenw | 0x00591ED8 | 0x191ED8 | int | 0x40F3C4 registration |
| screenh | 0x00591EDC | 0x191EDC | int | 0x40F3F4 registration |
| gamespeed | 0x00591F38 | 0x191F38 | int | 0x40F937 registration; percent |
| ispaused | 0x00591F3C | 0x191F3C | int | 0x40F967 registration of console variable paused |
| maxfps | 0x00591F24 | 0x191F24 | int | 0x40F8A7 registration |
| gamemode | 0x0058ABF8 | 0x18ABF8 | int | 0x409064 registration |
| guns | 0x0056BCC0 | 0x16BCC0 | guninfo[9] | 0x4CC085 etc.; stride 0x68 |
| server_clients_buf | 0x0058B0B8 | 0x18B0B8 | client** | 0x4948B5 followed by [eax+ebp*4] |
| server_clients_capacity | 0x0058B0BC | 0x18B0BC | int | Cube vector<T>: buf, alen, ulen |
| server_clients_length | 0x0058B0C0 | 0x18B0C0 | int | 0x4948A9 loop bounds |
| sg | 0x0056BC9C | 0x16BC9C | servergame* | 0x494EAE; gamemillis at sg+0x340 |

## Complete mapped player layout

Offsets are relative to playerent, whose observed allocation size is **0x408 bytes**. The layout combines physent, dynent, playerstate, and playerent. Multiple inheritance introduces a second vtable pointer at +0xE8.

The IDA header flattens inheritance and represents pointers as four-byte integers, independently of host pointer width. A vec3 is three consecutive floats; u8/i8 occupy one byte; u32/i32/f32 occupy four bytes. Padding gaps are intentional. An animstate is 20 bytes; poshist is 96 bytes.

The layout was reconstructed from the matching source and x86 ABI, with binary evidence from the constructor, allocation, and weapon-reference initialization. Every field was not individually exercised in gameplay.

| Field | Offset | Type | Count |
| --- | --- | --- | --- |
| vtable_physent | 0x000 | u32 | 1 |
| o | 0x004 | vec3 | 1 |
| vel | 0x010 | vec3 | 1 |
| deltapos | 0x01C | vec3 | 1 |
| newpos | 0x028 | vec3 | 1 |
| yaw | 0x034 | f32 | 1 |
| pitch | 0x038 | f32 | 1 |
| roll | 0x03C | f32 | 1 |
| pitchvel | 0x040 | f32 | 1 |
| maxspeed | 0x044 | f32 | 1 |
| timeinair | 0x048 | i32 | 1 |
| radius | 0x04C | f32 | 1 |
| eyeheight | 0x050 | f32 | 1 |
| maxeyeheight | 0x054 | f32 | 1 |
| aboveeye | 0x058 | f32 | 1 |
| inwater | 0x05C | u8 | 1 |
| onfloor | 0x05D | u8 | 1 |
| onladder | 0x05E | u8 | 1 |
| jumpnext | 0x05F | u8 | 1 |
| jumpd | 0x060 | u8 | 1 |
| crouching | 0x061 | u8 | 1 |
| crouchedinair | 0x062 | u8 | 1 |
| trycrouch | 0x063 | u8 | 1 |
| cancollide | 0x064 | u8 | 1 |
| stuck | 0x065 | u8 | 1 |
| scoping | 0x066 | u8 | 1 |
| lastjump | 0x068 | i32 | 1 |
| lastjumpheight | 0x06C | f32 | 1 |
| lastsplash | 0x070 | i32 | 1 |
| move | 0x074 | i8 | 1 |
| strafe | 0x075 | i8 | 1 |
| state | 0x076 | u8 | 1 |
| type | 0x077 | u8 | 1 |
| eyeheightvel | 0x078 | f32 | 1 |
| last_pos | 0x07C | i32 | 1 |
| k_left | 0x080 | u8 | 1 |
| k_right | 0x081 | u8 | 1 |
| k_up | 0x082 | u8 | 1 |
| k_down | 0x083 | u8 | 1 |
| prev | 0x084 | animstate | 2 |
| current | 0x0AC | animstate | 2 |
| lastanimswitchtime | 0x0D4 | i32 | 2 |
| lastmodel | 0x0DC | u32 | 2 |
| lastrendered | 0x0E4 | i32 | 1 |
| vtable_playerstate | 0x0E8 | u32 | 1 |
| health | 0x0EC | i32 | 1 |
| armour | 0x0F0 | i32 | 1 |
| primary | 0x0F4 | i32 | 1 |
| nextprimary | 0x0F8 | i32 | 1 |
| gunselect | 0x0FC | i32 | 1 |
| akimbo | 0x100 | u8 | 1 |
| ammo | 0x104 | i32 | 9 |
| mag | 0x128 | i32 | 9 |
| gunwait | 0x14C | i32 | 9 |
| pstatshots | 0x170 | i32 | 9 |
| pstatdamage | 0x194 | i32 | 9 |
| curskin | 0x1B8 | i32 | 1 |
| nextskin | 0x1BC | i32 | 2 |
| clientnum | 0x1C4 | i32 | 1 |
| lastupdate | 0x1C8 | i32 | 1 |
| plag | 0x1CC | i32 | 1 |
| ping | 0x1D0 | i32 | 1 |
| address | 0x1D4 | i32 | 1 |
| lifesequence | 0x1D8 | i32 | 1 |
| frags | 0x1DC | i32 | 1 |
| flagscore | 0x1E0 | i32 | 1 |
| deaths | 0x1E4 | i32 | 1 |
| tks | 0x1E8 | i32 | 1 |
| lastaction | 0x1EC | i32 | 1 |
| lastmove | 0x1F0 | i32 | 1 |
| lastpain | 0x1F4 | i32 | 1 |
| lastvoicecom | 0x1F8 | i32 | 1 |
| lastdeath | 0x1FC | i32 | 1 |
| clientrole | 0x200 | i32 | 1 |
| attacking | 0x204 | u8 | 1 |
| name | 0x205 | u8 | 260 |
| team | 0x30C | i32 | 1 |
| weaponchanging | 0x310 | i32 | 1 |
| nextweapon | 0x314 | i32 | 1 |
| spectatemode | 0x318 | i32 | 1 |
| followplayercn | 0x31C | i32 | 1 |
| eardamagemillis | 0x320 | i32 | 1 |
| maxroll | 0x324 | f32 | 1 |
| maxrolleffect | 0x328 | f32 | 1 |
| movroll | 0x32C | f32 | 1 |
| effroll | 0x330 | f32 | 1 |
| ffov | 0x334 | i32 | 1 |
| scopefov | 0x338 | i32 | 1 |
| weapons | 0x33C | u32 | 9 |
| prevweaponsel | 0x360 | u32 | 1 |
| weaponsel | 0x364 | u32 | 1 |
| nextweaponsel | 0x368 | u32 | 1 |
| primweap | 0x36C | u32 | 1 |
| nextprimweap | 0x370 | u32 | 1 |
| lastattackweapon | 0x374 | u32 | 1 |
| history | 0x378 | poshist | 1 |
| skin_noteam | 0x3D8 | u32 | 1 |
| skin_cla | 0x3DC | u32 | 1 |
| skin_rvsf | 0x3E0 | u32 | 1 |
| deltayaw | 0x3E4 | f32 | 1 |
| deltapitch | 0x3E8 | f32 | 1 |
| newyaw | 0x3EC | f32 | 1 |
| newpitch | 0x3F0 | f32 | 1 |
| smoothmillis | 0x3F4 | i32 | 1 |
| head | 0x3F8 | vec3 | 1 |
| ignored | 0x404 | u8 | 1 |
| muted | 0x405 | u8 | 1 |
| nocorpse | 0x406 | u8 | 1 |

## Weapon object layout

Read the selected weapon pointer from player + 0x364. The weapons pointer array begins at player + 0x33C.

C++ references are stored as addresses: weapon + 0x14 contains the address of the magazine counter, not its value:

    W = read_u32(P + 0x364)
    mag_address = read_u32(W + 0x14)
    mag_value = read_i32(mag_address)

The base weapon portion occupies 0x28 bytes; derived classes can be larger. Fields including shots and lastaction depend on initialization/reset behavior outside the base constructor.

| Field | Offset | Type |
| --- | --- | --- |
| vtable | 0x0 | u32 |
| type | 0x4 | i32 |
| owner | 0x8 | u32 |
| info | 0xC | u32 |
| ammo_ref | 0x10 | u32 |
| mag_ref | 0x14 | u32 |
| gunwait_ref | 0x18 | u32 |
| shots | 0x1C | i32 |
| reloading | 0x20 | i32 |
| lastaction | 0x24 | i32 |

## Weapon configuration: guninfo

The guns global is a direct array of nine records with a stride of **0x68 bytes**:

    guninfo_address = M + 0x16BCC0 + weapon_id*0x68

These are base configuration values. Effective recoil, spread, timing, and damage also depend on weapon logic and state. The shotgun damage member is not the total damage of a complete shot; consider pellet and bonus logic.

| Field | Offset | Type |
| --- | --- | --- |
| modelname | 0x0 | char[23] |
| title | 0x17 | char[42] |
| sound | 0x42 | short |
| reload | 0x44 | short |
| reloadtime | 0x46 | short |
| attackdelay | 0x48 | short |
| damage | 0x4A | short |
| piercing | 0x4C | short |
| projspeed | 0x4E | short |
| part | 0x50 | short |
| spread | 0x52 | short |
| recoil | 0x54 | short |
| magsize | 0x56 | short |
| mdl_kick_rot | 0x58 | short |
| mdl_kick_back | 0x5A | short |
| recoilincrease | 0x5C | short |
| recoilbase | 0x5E | short |
| maxrecoil | 0x60 | short |
| recoilbackfade | 0x62 | short |
| pushfactor | 0x64 | short |
| isauto | 0x66 | bool |

## Selected integrated-server client fields

These offsets are relative to a **server client pointer**, not the client-rendering playerent. Its embedded clientstate begins at +0x404. Server o and life state are distinct from client interpolation/render state. This is a selected field map, not a complete server-object declaration.

| Field | Offset | Type |
| --- | --- | --- |
| type | 0x0 | i32 |
| clientnum | 0x4 | i32 |
| clientsequence | 0x8 | i32 |
| peer | 0xC | u32 |
| hostname | 0x10 | char[260] |
| name | 0x290 | char[260] |
| team | 0x394 | i32 |
| state_base | 0x404 | playerstate |
| health | 0x408 | i32 |
| armour | 0x40C | i32 |
| primary | 0x410 | i32 |
| nextprimary | 0x414 | i32 |
| gunselect | 0x418 | i32 |
| akimbo | 0x41C | u8 |
| ammo | 0x420 | int[9] |
| mag | 0x444 | int[9] |
| gunwait | 0x468 | int[9] |
| pstatshots | 0x48C | int[9] |
| pstatdamage | 0x4B0 | int[9] |
| o | 0x4D4 | vec3 |
| state | 0x4E0 | i32 |
| lastdeath | 0x4E4 | i32 |
| lastspawn | 0x4E8 | i32 |
| spawn | 0x4EC | i32 |
| lifesequence | 0x4F0 | i32 |
| lastclaction | 0x4F4 | i32 |
| forced | 0x4F8 | u8 |
| lastshot | 0x4FC | i32 |
| akimbomillis | 0x524 | i32 |
| scoped | 0x528 | u8 |
| flagscore | 0x52C | i32 |
| frags | 0x530 | i32 |
| teamkills | 0x534 | i32 |
| deaths | 0x538 | i32 |
| events_buf | 0x564 | u32 |
| events_capacity | 0x568 | i32 |
| events_length | 0x56C | i32 |

## Useful enum values

| Enum | Values |
| --- | --- |
| Team | 0 CLA, 1 RVSF, 2 CLA spectator, 3 RVSF spectator, 4 spectator |
| Life state | 0 alive, 1 dead, 2 spawning, 3 lagged, 4 editing, 5 spectate |
| Entity type | 0 player, 1 bot, 2 camera, 3 bounce |
| Spectator mode | 0 none, 1 deathcam, 2 follow first person, 3 follow third person, 4 transparent third person, 5 fly, 6 overview |

Life state and entity type are **one-byte fields** at +0x76 and +0x77; do not read them as four-byte integers.

## Checking the offsets in Cheat Engine

Attach Cheat Engine to the matching executable. In **Add address manually**, enable **Pointer** and enter:

| Value | Base pointer address | Hexadecimal offset | Value type |
| --- | --- | --- | --- |
| Health | "ac_client.exe"+18AC00 | EC | 4 Bytes |
| Assault magazine | "ac_client.exe"+18AC00 | 140 | 4 Bytes |
| Assault reserve | "ac_client.exe"+18AC00 | 11C | 4 Bytes |
| Position X | "ac_client.exe"+18AC00 | 4 | Float |
| Yaw | "ac_client.exe"+18AC00 | 34 | Float |

Compare health with the display. Equip the assault rifle and fire once: the magazine should decrease by one. Reload and compare magazine/reserve changes. Move or turn to check position/angles. These are additional behavioral verification steps, not a claim that every step was performed during analysis.

Save the table and reattach after a restart. Module-relative pointer entries resolve the new player address automatically. The supplied table's XML and formulas were checked, but it was not opened in the Cheat Engine GUI.

### Finding a value's writer

Use **Find out what writes to this address**, trigger the action, and inspect the instruction and captured registers. For an indexed operand:

    mov [esi+ecx*4+444h], eax

The effective address is ESI + ECX*4 + 0x444. This displacement alone does not identify the object in ESI. Trace the register's origin and containing function before naming the field. Here, the matching source and server loop establish that it is a server client.

## Using IDA

Open the same executable, verify its hash, and allow analysis to complete. At the preferred base, press **G** and enter 00494F2A. For a rebased database:

    IDA address = IDA image base + 0x94F2A

For a runtime code address:

    RVA = runtime instruction address - runtime module base
    IDA address = IDA image base + RVA

Search for the binary byte sequence 89 84 8E 44 04 00 00, with a range/direction covering the code. Text searches for disassembly are different. Rebasing changes addresses but does not by itself change this instruction's bytes.

Import the IDA header into Local Types and apply ACPlayerEnt32 to the relevant player pointer/structure operand. Use the server field map for the instruction above: applying the player layout to that ESI value would mislabel it.

Optionally run the annotation script with **File > Script file**. Keep it beside offsets.json and ida_types.h. It verifies the executable hash and known instruction, imports types, and adds prefixed names/comments. It does not patch code bytes. Python syntax was checked; execution inside the user's IDA installation was not tested.

## Validation and confidence

Evidence supporting the map:

1. The routine newplayerent() at 0x0047EF70 allocates 0x408 bytes and calls the constructor at 0x0041C520.
2. The constructor initializes both vtable pointers and relevant fields, including invalid head coordinates.
3. Weapon initialization sets each weapon's owner, ID, configuration pointer, and references to the expected ammo, mag, and gunwait entries.
4. Runtime sampling confirmed those reference relationships for all nine weapon objects.
5. OpenGL calls identify modelview/projection storage. Numerical checks confirmed projection times modelview equals MVP, both inverse relationships, and the aspect/FOV interpretation.
6. The server loop identifies its client vector and shot handler. The sampled local-server client had health and assault-magazine values consistent with its client-side counterpart.

All **16 recorded runtime checks** passed. The maximum absolute error for projection times modelview versus stored MVP was approximately 2.07e-6. Compiled C++ assertions checked the size and offsets of all 108 mapped player fields, plus the base weapon size.

Limitations:

- The other-player vector was empty. Remote entities and animated head updates have static/source support, without bot-based runtime validation in that sample.
- Checking weapon pointers/references does not mean firing and reloading every weapon was behaviorally tested.
- Runtime reads were not synchronized with rendering.
- The Cheat Engine table and IDA script have structural/syntax checks, not GUI execution coverage.
- Unlisted functions, structures, and data were not exhaustively reversed. Build changes can invalidate the map.

## Sources

Names and semantics were cross-checked against the matching official tag:

- [Official v1.3.0.2 source](https://github.com/assaultcube/AC/tree/v1.3.0.2).
- [entity.h: physics, playerstate, playerent, and enums](https://github.com/assaultcube/AC/blob/v1.3.0.2/source/src/entity.h).
- [weapon.h](https://github.com/assaultcube/AC/blob/v1.3.0.2/source/src/weapon.h) and [weapon.cpp: weapon references and firing](https://github.com/assaultcube/AC/blob/v1.3.0.2/source/src/weapon.cpp).
- [rendergl.cpp: camera and matrices](https://github.com/assaultcube/AC/blob/v1.3.0.2/source/src/rendergl.cpp).
- [rendermodel.cpp: tag_head updates](https://github.com/assaultcube/AC/blob/v1.3.0.2/source/src/rendermodel.cpp).
- [clientgame.cpp: player1 and player list](https://github.com/assaultcube/AC/blob/v1.3.0.2/source/src/clientgame.cpp).
- [tools.h: vector and string definitions](https://github.com/assaultcube/AC/blob/v1.3.0.2/source/src/tools.h).
- [server.h: server layout and guninfo](https://github.com/assaultcube/AC/blob/v1.3.0.2/source/src/server.h).
- [serverevents.h: server ammunition consumption](https://github.com/assaultcube/AC/blob/v1.3.0.2/source/src/serverevents.h).
- [main.cpp: window size and timing variables](https://github.com/assaultcube/AC/blob/v1.3.0.2/source/src/main.cpp).
- [Cheat Engine: finding writes/accesses](https://www.cheatengine.org/help/find-out-what-writesaccesses-this-address.htm).
- [Hex-Rays: structure offsets](https://docs.hex-rays.com/8.4/user-guide/disassembler/structures-tutorial) and [IDAPython parse_decls](https://python.docs.hex-rays.com/idc/index.html#idc.parse_decls).

This repository is independent documentation, unaffiliated with the AssaultCube project. No game executable is distributed here.
