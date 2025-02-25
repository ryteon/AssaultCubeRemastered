// creation of scoreboard pseudo-menu

#include "cube.h"
#define SCORERATIO(F,D) (float)(F >= 0 ? F : 0) / (float)(D > 0 ? D : 1)

void *scoremenu = NULL;
bool needscoresreorder = true;

void showscores(bool on)
{
    if(on) showmenu("score", false);
    else closemenu("score");
}

COMMANDF(showscores, "i", (int *on) { showscores(*on != 0); });

VARFP(sc_flags,      0,  0, 100, needscoresreorder = true);
VARFP(sc_frags,      0,  1, 100, needscoresreorder = true);
VARFP(sc_deaths,    -1,  2, 100, needscoresreorder = true);
VARFP(sc_ratio,     -1, -1, 100, needscoresreorder = true);
VARFP(sc_lag,       -1,  4, 100, needscoresreorder = true);
VARFP(sc_clientnum,  0,  5, 100, needscoresreorder = true);
VARFP(sc_name,       0,  6, 100, needscoresreorder = true);

struct coldata
{
    int priority;
    char *val;

    coldata() : priority(-1), val(NULL) {}
    ~coldata()
    {
        DELETEA(val);
    }
};

// FIXME ? if two columns share teh same priority
// they will be sorted by the order they were added with addcol
int sortcolumns(coldata *col_a, coldata *col_b)
{
    if(col_a->priority > col_b->priority) return 1;
    else if(col_a->priority < col_b->priority) return -1;
    return 0;
}

struct sline
{
    string s;
    color *bgcolor;
    char textcolor;
    vector<coldata> cols;

    sline() : bgcolor(NULL), textcolor(0) { copystring(s, ""); }

    void addcol(int priority, const char *format = NULL, ...) PRINTFARGS(3, 4)
    {
        if(priority < 0) return;
        coldata &col = cols.add();
        col.priority = priority;
        if(format && *format)
        {
            defvformatstring(sf, format, format);
            col.val = newstring(sf);
        }
    }

    char *getcols()
    {
        if(s[0] == '\0')
        {
            if(textcolor) formatstring(s)("\f%c", textcolor);
            cols.sort(sortcolumns);
            loopv(cols)
            {
               if(i > 0) concatstring(s, "\t");
               if(cols[i].val) concatstring(s, cols[i].val);
            }
        }
        return s;
    }
};

static vector<sline> scorelines;
vector<discscore> discscores;

struct teamscore
{
    int team, frags, deaths, flagscore;
    vector<playerent *> teammembers;
    teamscore(int t) : team(t), frags(0), deaths(0), flagscore(0) {}

    void addplayer(playerent *d)
    {
        if(!d) return;
        teammembers.add(d);
        frags += d->frags;
        deaths += d->deaths;
        if(m_flags_) flagscore += d->flagscore;
    }

    void addscore(discscore &d)
    {
        frags += d.frags;
        deaths += d.deaths;
        if(m_flags_) flagscore += d.flags;
    }
};

void calcteamscores(int scores[4])
{
    teamscore teamscores[2] = { teamscore(TEAM_T), teamscore(TEAM_CT) };
    loopv(players) if(players[i] && players[i]->team != TEAM_SPECT)
    {
        teamscores[team_base(players[i]->team)].addplayer(players[i]);
    }
    loopv(discscores) if(discscores[i].team != TEAM_SPECT)
    {
        teamscores[team_base(discscores[i].team)].addscore(discscores[i]);
    }
    if(!watchingdemo && player1->team != TEAM_SPECT) teamscores[team_base(player1->team)].addplayer(player1);
    loopi(2)
    {
        scores[i] = teamscores[i].flagscore;
        scores[i + 2] = teamscores[i].frags;
    }
}

static int teamscorecmp(const teamscore *x, const teamscore *y)
{
    if(x->flagscore > y->flagscore) return -1;
    if(x->flagscore < y->flagscore) return 1;
    if(x->frags > y->frags) return -1;
    if(x->frags < y->frags) return 1;
    if(x->deaths < y->deaths) return -1;
    return 0;
}

static int scorecmp(playerent **x, playerent **y)
{
    if((*x)->flagscore > (*y)->flagscore) return -1;
    if((*x)->flagscore < (*y)->flagscore) return 1;
    if((*x)->frags > (*y)->frags) return -1;
    if((*x)->frags < (*y)->frags) return 1;
    if((*x)->deaths > (*y)->deaths) return 1;
    if((*x)->deaths < (*y)->deaths) return -1;
    if((*x)->lifesequence > (*y)->lifesequence) return 1;
    if((*x)->lifesequence < (*y)->lifesequence) return -1;
    return 0;
}

static int discscorecmp(const discscore *x, const discscore *y)
{
    if(x->team < y->team) return -1;
    if(x->team > y->team) return 1;
    if(m_flags_ && x->flags > y->flags) return -1;
    if(m_flags_ && x->flags < y->flags) return 1;
    if(x->frags > y->frags) return -1;
    if(x->frags < y->frags) return 1;
    if(x->deaths > y->deaths) return 1;
    if(x->deaths < y->deaths) return -1;
    return strcmp(x->name, y->name);
}

// const char *scoreratio(int frags, int deaths, int precis = 0)
// {
//     static string res;
//     float ratio = SCORERATIO(frags, deaths);
//     int precision = precis;
//     if(!precision)
//     {
//         if(ratio<10.0f) precision = 2;
//         else if(ratio<100.0f) precision = 1;
//     }
//     formatstring(res)("%.*f", precision, ratio);
//     return res;
// }

void renderdiscscores(int team)
{
    loopv(discscores) if(team == team_group(discscores[i].team))
    {
        discscore &d = discscores[i];
        sline &line = scorelines.add();
        if(team_isspect(d.team)) line.textcolor = '4';
        const char *clag = team_isspect(d.team) ? "SPECT" : "";

        if(m_flags_) line.addcol(sc_flags, "%d", d.flags);
        line.addcol(sc_frags, "%d", d.frags);
        line.addcol(sc_deaths, "%d", d.deaths);
        line.addcol(sc_ratio, "%.2f", SCORERATIO(d.frags, d.deaths));
        line.addcol(sc_lag, "%s", clag);
        line.addcol(sc_clientnum, "DISC");
        line.addcol(sc_name, "%s", d.name);
    }
}

VARP(cncolumncolor, 0, 5, 9);

void renderscore(playerent *d)
{
    const char *status = "";
    string lagping;
    static color localplayerc(0.2f, 0.2f, 0.2f, 0.2f);
    if(d->clientrole==CR_ADMIN) status = d->state!=CS_ALIVE ? "\f7" : "\f3";
    else if(d->state!=CS_ALIVE) status = "\f4";
    if (team_isspect(d->team)) copystring(lagping, "SPECT");
    else if (d->state==CS_LAGGED || (d->ping > 999 && d->plag > 99)) copystring(lagping, "LAG");
    else
    {
        if(multiplayer(NULL)) formatstring(lagping)("%s/%s", colorpj(d->plag), colorping(d->ping));
        else formatstring(lagping)("%d/%d", d->plag, d->ping);
    }
    const char *ign = d->ignored ? " (ignored)" : (d->muted ? " (muted)" : "");
    sline &line = scorelines.add();
    if(team_isspect(d->team)) line.textcolor = '4';
    line.bgcolor = d==player1 ? &localplayerc : NULL;

    if(m_flags_) line.addcol(sc_flags, "%d", d->flagscore);
    line.addcol(sc_frags, "%d", d->frags);
    line.addcol(sc_deaths, "%d", d->deaths);
    line.addcol(sc_ratio, "%.2f", SCORERATIO(d->frags, d->deaths));
    if(multiplayer(NULL) || watchingdemo) line.addcol(sc_lag, "%s", lagping);

    line.addcol(sc_clientnum, "\fs\f%d%d\fr", cncolumncolor, d->clientnum);
    char flagicon = '\0';
    if(m_flags_) //show flag icon at flag carrier with use radaricons font
    {
        loopi(2)
        {
            if(flaginfos[i].state == CTFF_STOLEN && flaginfos[i].actor == d) flagicon = m_ktf ? 'L' : "DH"[i];
        }
    }
    line.addcol(sc_name, "\fs%s%s\fr%s%s%c ", status, colorname(d), ign, flagicon ? " \a" : "", flagicon);
}

int totalplayers = 0;

void renderteamscore(teamscore *t)
{
    if(!scorelines.empty()) // space between teams
    {
        sline &space = scorelines.add();
        space.s[0] = 0;
    }
    sline &line = scorelines.add();

    if(m_flags_) line.addcol(sc_flags, "%d", t->flagscore);
    line.addcol(sc_frags, "%d", t->frags);
    line.addcol(sc_deaths, "%d", t->deaths);
    line.addcol(sc_ratio, "%.2f", SCORERATIO(t->frags, t->deaths));
    if(multiplayer(NULL) || watchingdemo) line.addcol(sc_lag);
    line.addcol(sc_clientnum, "%s", team_string(t->team));
    int n = t->teammembers.length();
    line.addcol(sc_name, "(%d %s)", n, n == 1 ? "player" : "players");

    static color teamcolors[2] = { color(1.0f, 0, 0, 0.2f), color(0, 0, 1.0f, 0.2f) };
    line.bgcolor = &teamcolors[team_base(t->team)];
    loopv(t->teammembers) renderscore(t->teammembers[i]);
}

extern bool watchingdemo;

void reorderscorecolumns()
{
    static string scoreboardtitle;
    needscoresreorder = false;
    extern void *scoremenu;
    sline sscore;

    if(m_flags_) sscore.addcol(sc_flags, "flags");
    sscore.addcol(sc_frags, "frags");
    sscore.addcol(sc_deaths, "deaths");
    sscore.addcol(sc_ratio, "ratio");
    if(multiplayer(NULL) || watchingdemo) sscore.addcol(sc_lag, "pj/ping");
    sscore.addcol(sc_clientnum, "cn");
    sscore.addcol(sc_name, "name");
    copystring(scoreboardtitle, sscore.getcols());
    menutitlemanual(scoremenu, scoreboardtitle);
}

void renderscores(void *menu, bool init)
{
    if(needscoresreorder) reorderscorecolumns();
    static string modeline, serverline;

    modeline[0] = '\0';
    serverline[0] = '\0';
    scorelines.shrink(0);

    vector<playerent *> scores;
    if(!watchingdemo) scores.add(player1);
    totalplayers = 1;
    loopv(players) if(players[i]) { scores.add(players[i]); totalplayers++; }
    scores.sort(scorecmp);
    discscores.sort(discscorecmp);

    int spectators = 0;
    loopv(scores) if(scores[i]->team == TEAM_SPECT) spectators++;
    loopv(discscores) if(discscores[i].team == TEAM_SPECT) spectators++;

    int winner = -1;
    if(m_teammode)
    {
        teamscore teamscores[2] = { teamscore(TEAM_T), teamscore(TEAM_CT) };

        loopv(scores) if(scores[i]->team != TEAM_SPECT) teamscores[team_base(scores[i]->team)].addplayer(scores[i]);
        loopv(discscores) if(discscores[i].team != TEAM_SPECT) teamscores[team_base(discscores[i].team)].addscore(discscores[i]);

        int sort = teamscorecmp(&teamscores[TEAM_T], &teamscores[TEAM_CT]) < 0 ? 0 : 1;
        loopi(2)
        {
            renderteamscore(&teamscores[sort ^ i]);
            renderdiscscores(sort ^ i);
        }
        winner = m_flags_ ?
            (teamscores[sort].flagscore > teamscores[team_opposite(sort)].flagscore ? sort : -1) :
            (teamscores[sort].frags > teamscores[team_opposite(sort)].frags ? sort : -1);

    }
    else
    { // ffa mode
        loopv(scores) if(scores[i]->team != TEAM_SPECT) renderscore(scores[i]);
        loopi(2) renderdiscscores(i);
        if(scores.length() > 0)
        {
            winner = scores[0]->clientnum;
            if(scores.length() > 1
                && ((m_flags_ && scores[0]->flagscore == scores[1]->flagscore)
                     || (!m_flags_ && scores[0]->frags == scores[1]->frags)))
                winner = -1;
        }
    }
    if(spectators)
    {
        if(!scorelines.empty()) // space between teams and spectators
        {
            sline &space = scorelines.add();
            space.s[0] = 0;
        }
        renderdiscscores(TEAM_SPECT);
        loopv(scores) if(scores[i]->team == TEAM_SPECT) renderscore(scores[i]);
    }

    if(getclientmap()[0])
    {
        bool fldrprefix = !strncmp(getclientmap(), "maps/", strlen("maps/"));
        formatstring(modeline)("\"%s\" on map %s", modestr(gamemode, modeacronyms > 0), fldrprefix ? getclientmap()+strlen("maps/") : getclientmap());
    }

    extern int minutesremaining, gametimedisplay;
    extern string gtime;

    if((gamemode>1 || (gamemode==0 && (multiplayer(NULL) || watchingdemo))) && minutesremaining >= 0)
    {
        if(!minutesremaining)
        {
            concatstring(modeline, ", intermission");

            if (m_teammode) // Add in the winning team
            {
                switch(winner)
                {
                    case TEAM_T: concatstring(modeline, ", \f3Terrorists win!"); break;
                    case TEAM_CT: concatstring(modeline, ", \f1Counter-Terrorists win!"); break;
                    case -1:
                    default:
                        concatstring(modeline, ", \f2it's a tie!");
                    break;
                }
            }
            else // Add the winning player
            {
                if (winner < 0) concatstring(modeline, ", \f2it's a tie!");
                else concatformatstring(modeline, ", \f1%s wins!", scores[0]->name);
            }
        }
        else
        {
            concatformatstring(modeline, ", %s", gtime);
            if(gametimedisplay == 2) concatformatstring(modeline, " / %d:00", gametimemaximum/60000);
            else concatformatstring(modeline, " remaining");
        }
    }

    if(multiplayer(NULL))
    {
        serverinfo *s = getconnectedserverinfo();
        if(s)
        {
            if(servstate.mastermode > MM_OPEN)
            {
                if(servstate.mastermode == MM_MATCH) concatformatstring(serverline, "M%d ", servstate.matchteamsize);
                else concatstring(serverline, "P ");
            }
            // ft: 2010jun12: this can write over the menu boundary
            //concatformatstring(serverline, "%s:%d %s", s->name, s->port, s->sdesc);
            // for now we'll just cut it off, same as the serverbrowser
            // but we might want to consider wrapping the bottom-line to accomodate longer descriptions - to a limit.
            string text;
            filtertext(text, s->sdesc, FTXT__SERVDESC);
            //for(char *p = text; (p = strchr(p, '\"')); *p++ = ' ');
            //text[30] = '\0'; // serverbrowser has less room - +8 chars here - 2010AUG03 - seems it was too much, falling back to 30 (for now): TODO get real width of menu as reference-width. FIXME: cutoff
            concatformatstring(serverline, "%s:%d %s", s->name, s->port, text);
            //printf("SERVERLINE: %s\n", serverline);
        }
    }

    menureset(menu);
    loopv(scorelines) menuimagemanual(menu, NULL, "radaricons", scorelines[i].getcols(), NULL, scorelines[i].bgcolor);
    menuheader(menu, modeline, serverline);

    // update server stats
    static int lastrefresh = 0;
    if(multiplayer(NULL) && (!lastrefresh || lastrefresh + 5000 < lastmillis))
    {
        refreshservers(NULL, init);
        lastrefresh = lastmillis;
    }
}

#define MAXJPGCOM 65533  // maximum JPEG comment length

static void addstr(char *&dest, const char *end, const char *src) { size_t l = strlen(src); if(dest + l < end) memcpy(dest, src, l + 1), dest += l; }

const char *asciiscores(bool destjpg)
{
    static char *buf = NULL;
    static string team, flags, text;
    playerent *d;
    vector<playerent *> scores;

    if(!buf) buf = (char *) malloc(MAXJPGCOM +1);
    if(!buf) return "";

    if(!watchingdemo) scores.add(player1);
    loopv(players) if(players[i]) scores.add(players[i]);
    scores.sort(scorecmp);

    copystring(buf, "Comment", MAXJPGCOM);              // include PNG keyword and skip it (which looks like a hack because it is one)
    char *t = buf + 8, *e = buf + MAXJPGCOM;
    if(destjpg)
    {
        formatstring(text)("AssaultCube Screenshot (%s)\n", asctimestr());
        addstr(t, e, text);
    }
    if(getclientmap()[0])
    {
        formatstring(text)("\n\"%s\" on map %s", modestr(gamemode, 0), getclientmap());
        addstr(t, e, text);
    }
    if(multiplayer(NULL))
    {
        serverinfo *s = getconnectedserverinfo();
        if(s)
        {
            string sdesc;
            filtertext(sdesc, s->sdesc, FTXT__SERVDESC | FTXT_NOCOLOR);
            formatstring(text)(", %s:%d %s", s->name, s->port, sdesc);
            addstr(t, e, text);
        }
    }
    if(destjpg)
        addstr(t, e, "\n");
    else
    {
        formatstring(text)("\n%sfrags deaths cn%s name\n", m_flags_ ? "flags " : "", m_teammode ? " team" : "");
        addstr(t, e, text);
    }
    loopv(scores)
    {
        d = scores[i];
//         const char *sr = scoreratio(d->frags, d->deaths);
        formatstring(team)(destjpg ? ", %s" : " %-4s", team_string(d->team, true));
        formatstring(flags)(destjpg ? "%d/" : " %4d ", d->flagscore);
        if(destjpg)
            formatstring(text)("%s%s (%s%d/%d)\n", d->name, m_teammode ? team : "", m_flags_ ? flags : "", d->frags, d->deaths);
        else
            formatstring(text)("%s %4d   %4d %2d%s %s%s\n", m_flags_ ? flags : "", d->frags, d->deaths, d->clientnum,
                            m_teammode ? team : "", d->name, d->clientrole==CR_ADMIN ? " (admin)" : d==player1 ? " (you)" : "");
        addstr(t, e, text);
    }
    discscores.sort(discscorecmp);
    loopv(discscores)
    {
        discscore &d = discscores[i];
//         const char *sr = scoreratio(d.frags, d.deaths);
        formatstring(team)(destjpg ? ", %s" : " %-4s", team_string(d.team, true));
        formatstring(flags)(destjpg ? "%d/" : " %4d ", d.flags);
        if(destjpg)
            formatstring(text)("%s(disconnected)%s (%s%d/%d)\n", d.name, m_teammode ? team : "", m_flags_ ? flags : "", d.frags, d.deaths);
        else
            formatstring(text)("%s %4d   %4d --%s %s(disconnected)\n", m_flags_ ? flags : "", d.frags, d.deaths, m_teammode ? team : "", d.name);
        addstr(t, e, text);
    }
    if(destjpg)
    {
        extern int minutesremaining;
        formatstring(text)("(%sfrags/deaths), %d minute%s remaining\n", m_flags_ ? "flags/" : "", minutesremaining, minutesremaining == 1 ? "" : "s");
        addstr(t, e, text);
    }
    return buf + 8;
}

void consolescores()
{
    printf("%s\n", asciiscores());
}

void winners()
{
    string winners = "";
    vector<playerent *> scores;
    if(!watchingdemo) scores.add(player1);
    loopv(players) if(players[i]) { scores.add(players[i]); }
    scores.sort(scorecmp);
    discscores.sort(discscorecmp);

    if(m_teammode)
    {
        teamscore teamscores[2] = { teamscore(TEAM_T), teamscore(TEAM_CT) };

        loopv(scores) if(scores[i]->team != TEAM_SPECT) teamscores[team_base(scores[i]->team)].addplayer(scores[i]);
        loopv(discscores) if(discscores[i].team != TEAM_SPECT)
        teamscores[team_base(discscores[i].team)].addscore(discscores[i]);

        int sort = teamscorecmp(&teamscores[TEAM_T], &teamscores[TEAM_CT]);
        if(!sort) copystring(winners, "0 1");
        else itoa(winners, sort < 0 ? 0 : 1);
    }
    else
    {
        loopv(scores)
        {
            if(!i || !scorecmp(&scores[i], &scores[i-1])) concatformatstring(winners, "%s%d", i ? " " : "", scores[i]->clientnum);
            else break;
        }
    }

    result(winners);
}

COMMAND(winners, "");
