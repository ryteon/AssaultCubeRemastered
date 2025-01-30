#pragma once

enum                            // block types
{
    SOLID = 0,                  // entirely solid cube [only specifies wtex]
    CORNER,                     // half full corner of a wall
    FHF,                        // floor heightfield using neighbour vdelta values
    CHF,                        // idem ceiling
    SPACE,                      // entirely empty cube
    SEMISOLID,                  // generated by mipmapping
    MAXTYPE
};

struct sqr
{
    uchar type;                 // one of the above
    char floor, ceil;           // height, in cubes
    uchar wtex, ftex, ctex;     // wall/floor/ceil texture ids
    uchar r, g, b;              // light value at upper left vertex
    uchar vdelta;               // vertex delta, used for heightfield cubes
    char defer;                 // used in mipmapping, when true this cube is not a perfect mip
    char occluded;              // true when occluded
    uchar utex;                 // upper wall tex id
    uchar tag;                  // used by triggers
    uchar visible;              // temporarily used to flag the visibility of a cube (INVISWTEX, INVISUTEX, INVISIBLE)
    uchar reserved;
};

struct servsqr                  // server variant of sqr
{
    uchar type;                 // SOLID, etc.  (also contains the two tagclip bits ORed in)
    char floor, ceil;           // height, in cubes
    uchar vdelta;               // vertex delta, used for heightfield cubes
};

enum                            // hardcoded texture numbers
{
    DEFAULT_SKY = 0,
    DEFAULT_LIQUID,
    DEFAULT_WALL,
    DEFAULT_FLOOR,
    DEFAULT_CEIL
};

enum                            // stuff encoded in sqr.tag
{
    TAGTRIGGERMASK = 0x3F,      // room for old fashioned cube 1 triggers
    TAGCLIP = 0x40,             // clips all objects
    TAGPLCLIP = 0x80            // clips only players
};
#define TAGANYCLIP (TAGCLIP|TAGPLCLIP)

enum
{
    INVISWTEX = 1<<0,
    INVISUTEX = 1<<1,
    INVISIBLE = 1<<2
};

enum
{
    MHF_AUTOMAPCONFIG = 1<<0,             // autogenerate map-config during map save
    MHF_DISABLEWATERREFLECT = 1 << 8,     // force waterreflect to zero
    MHF_LIMITWATERWAVEHEIGHT = 1 << 9,    // limit waveheight to 0.1
    MHF_DISABLESTENCILSHADOWS = 1 << 10   // force stencilshadow to 0
};

#define MAPVERSION 10           // default map format version to be written (bump if map format changes, see worldio.cpp)

struct header                   // map file format header
{
    char head[4];               // "CUBE"
    int version;                // any >8bit quantity is little endian
    int headersize;             // sizeof(header)
    int sfactor;                // in bits
    int numents;
    char maptitle[128];
    uchar texlists[3][256];
    int waterlevel;
    uchar watercolor[4];
    int maprevision;
    int ambient;
    int flags;                  // MHF_*
    int timestamp;              // UTC unixtime of time of save (yes, this will break in 2038)
    int reserved[10];
    //char mediareq[128];         // version 7 and 8 only.
};

struct mapsoundline { string name; int maxuses; };

struct _mapconfigdata
{
    string notexturename;
    vector<mapsoundline> mapsoundlines;
    void clear()
    {
        *notexturename = '\0';
        mapsoundlines.shrink(0);
    }
};

struct mapdim_s
{   //   0   2   1   3     6         7
    int x1, x2, y1, y2, minfloor, maxceil;       // outer borders (points to last used cube)
    //    4      5
    int xspan, yspan;                            // size of area between x1|y1 and x2|y2
    float xm, ym;                                // middle of the map
};

#define MAS_VDELTA_QUANT 8
#define MAS_VDELTA_TABSIZE 5            // (hardcoded maximum of 15!)
#define MAS_VDELTA_THRES 3
#define MAS_GRID 8
#define MAS_GRID2 (MAS_GRID * MAS_GRID)
#define MAS_RESOLUTION 32               // rays (32 * 8)
struct mapareastats_s
{
    int vdd[MAS_VDELTA_TABSIZE];        // vdelta-deltas (detect ugly steep heightfields)
    int vdds;                           // vdd table reduced to an easy to check bitmask
    int ppa[MAS_GRID2];                 // area visible per probe point
    int ppv[MAS_GRID2];                 // volume visible per probe point
    int total;                          // number of non-solid cubes on the map
    int rest;                           // area not visible from any probe point
    #ifndef STANDALONE
    int steepest;                       // location of steepest vdelta
    int ppp[MAS_GRID2];                 // probe point position (client-only)
    #endif
};

#define SWS(w,x,y,s) (&(w)[((y)<<(s))+(x)])
#define SW(w,x,y) SWS(w,x,y,sfactor)
#define S(x,y) SW(world,x,y)            // convenient lookup of a lowest mip cube
#define SMALLEST_FACTOR 6               // determines number of mips there can be
#define DEFAULT_FACTOR 8
#define LARGEST_FACTOR 11               // 10 is already insane
#define MAXENTITIES 65535
#define MAXHEADEREXTRA (1<<20)
#define SOLID(x) ((x)->type==SOLID)
#define MINBORD 2                       // 2 cubes from the edge of the world are always solid
#define OUTBORD(x,y) ((x)<MINBORD || (y)<MINBORD || (x)>=ssize-MINBORD || (y)>=ssize-MINBORD)
#define OUTBORDRAD(x,y,rad) (int(x-rad)<MINBORD || int(y-rad)<MINBORD || int(x+rad)>=ssize-MINBORD || (y+rad)>=ssize-MINBORD)
#define WATERLEVELSCALING 10

struct block { int x, y, xs, ys, h; short p[5]; };

// vertex array format

struct vertex { float u, v, x, y, z; uchar r, g, b, a; };

// map statistics

#define MINSPAWNS 5 // minimum number of spawns per team
#define MINFLAGDISTANCE 24 // w/o checking for walls between them // minimum flag entity distance (2D)

struct entitystats_s
{
    int entcnt[MAXENTTYPES];                // simple count for every basic ent type
    int spawns[3];                          // number of spawns with valid attr2
    int flags[2];                           // number of flags with valid attr2
    int flagents[2];                        // entity indices of the flag ents
    int pickups;                            // total number of pickup entities
    int pickupdistance[LARGEST_FACTOR + 1]; // entity distance values grouped logarithmically
    int flagentdistance;                    // distance of the two flag entities, if existing
    int modes_possible;                     // bitmask of possible gamemodes, depending on flag and spawn entities
    bool hasffaspawns;                      // has enough ffa spawns
    bool hasteamspawns;                     // has enough team spawns
    bool hasflags;                          // has exactly 2 valid flags
    int unknownspawns, unknownflags;        // attr2 is not valid
    int firstclip;                          // index of first clipped entity
    short first[MAXENTTYPES], last[MAXENTTYPES]; // first and last occurence of every basic ent type
};

