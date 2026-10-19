typedef unsigned int AC_PTR32;
typedef struct { float x,y,z; } ACVec3;
typedef struct { int anim,frame,range,basetime; float speed; } ACAnimState;
typedef struct { int nextupdate,curpos,numpos; ACVec3 pos[7]; } ACPosHist;
typedef struct ACPlayerEnt32 {
    unsigned int vtable_physent; // 0x000
    ACVec3 o; // 0x004
    ACVec3 vel; // 0x010
    ACVec3 deltapos; // 0x01C
    ACVec3 newpos; // 0x028
    float yaw; // 0x034
    float pitch; // 0x038
    float roll; // 0x03C
    float pitchvel; // 0x040
    float maxspeed; // 0x044
    int timeinair; // 0x048
    float radius; // 0x04C
    float eyeheight; // 0x050
    float maxeyeheight; // 0x054
    float aboveeye; // 0x058
    unsigned char inwater; // 0x05C
    unsigned char onfloor; // 0x05D
    unsigned char onladder; // 0x05E
    unsigned char jumpnext; // 0x05F
    unsigned char jumpd; // 0x060
    unsigned char crouching; // 0x061
    unsigned char crouchedinair; // 0x062
    unsigned char trycrouch; // 0x063
    unsigned char cancollide; // 0x064
    unsigned char stuck; // 0x065
    unsigned char scoping; // 0x066
    unsigned char padding_67[1];
    int lastjump; // 0x068
    float lastjumpheight; // 0x06C
    int lastsplash; // 0x070
    signed char move; // 0x074
    signed char strafe; // 0x075
    unsigned char state; // 0x076
    unsigned char type; // 0x077
    float eyeheightvel; // 0x078
    int last_pos; // 0x07C
    unsigned char k_left; // 0x080
    unsigned char k_right; // 0x081
    unsigned char k_up; // 0x082
    unsigned char k_down; // 0x083
    ACAnimState prev[2]; // 0x084
    ACAnimState current[2]; // 0x0AC
    int lastanimswitchtime[2]; // 0x0D4
    unsigned int lastmodel[2]; // 0x0DC
    int lastrendered; // 0x0E4
    unsigned int vtable_playerstate; // 0x0E8
    int health; // 0x0EC
    int armour; // 0x0F0
    int primary; // 0x0F4
    int nextprimary; // 0x0F8
    int gunselect; // 0x0FC
    unsigned char akimbo; // 0x100
    unsigned char padding_101[3];
    int ammo[9]; // 0x104
    int mag[9]; // 0x128
    int gunwait[9]; // 0x14C
    int pstatshots[9]; // 0x170
    int pstatdamage[9]; // 0x194
    int curskin; // 0x1B8
    int nextskin[2]; // 0x1BC
    int clientnum; // 0x1C4
    int lastupdate; // 0x1C8
    int plag; // 0x1CC
    int ping; // 0x1D0
    int address; // 0x1D4
    int lifesequence; // 0x1D8
    int frags; // 0x1DC
    int flagscore; // 0x1E0
    int deaths; // 0x1E4
    int tks; // 0x1E8
    int lastaction; // 0x1EC
    int lastmove; // 0x1F0
    int lastpain; // 0x1F4
    int lastvoicecom; // 0x1F8
    int lastdeath; // 0x1FC
    int clientrole; // 0x200
    unsigned char attacking; // 0x204
    char name[260]; // 0x205
    unsigned char padding_309[3];
    int team; // 0x30C
    int weaponchanging; // 0x310
    int nextweapon; // 0x314
    int spectatemode; // 0x318
    int followplayercn; // 0x31C
    int eardamagemillis; // 0x320
    float maxroll; // 0x324
    float maxrolleffect; // 0x328
    float movroll; // 0x32C
    float effroll; // 0x330
    int ffov; // 0x334
    int scopefov; // 0x338
    unsigned int weapons[9]; // 0x33C
    unsigned int prevweaponsel; // 0x360
    unsigned int weaponsel; // 0x364
    unsigned int nextweaponsel; // 0x368
    unsigned int primweap; // 0x36C
    unsigned int nextprimweap; // 0x370
    unsigned int lastattackweapon; // 0x374
    ACPosHist history; // 0x378
    unsigned int skin_noteam; // 0x3D8
    unsigned int skin_cla; // 0x3DC
    unsigned int skin_rvsf; // 0x3E0
    float deltayaw; // 0x3E4
    float deltapitch; // 0x3E8
    float newyaw; // 0x3EC
    float newpitch; // 0x3F0
    int smoothmillis; // 0x3F4
    ACVec3 head; // 0x3F8
    unsigned char ignored; // 0x404
    unsigned char muted; // 0x405
    unsigned char nocorpse; // 0x406
    unsigned char padding_407[1];
} ACPlayerEnt32;
typedef struct ACWeapon32 {
    unsigned int vtable; // 0x0
    int type; // 0x4
    unsigned int owner; // 0x8
    unsigned int info; // 0xC
    unsigned int ammo_ref; // 0x10
    unsigned int mag_ref; // 0x14
    unsigned int gunwait_ref; // 0x18
    int shots; // 0x1C
    int reloading; // 0x20
    int lastaction; // 0x24
} ACWeapon32;
