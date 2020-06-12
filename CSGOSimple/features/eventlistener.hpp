//#pragma once
//#include "../options.hpp"
//#include "../valve_sdk/csgostructs.hpp"
//#include "../helpers/math.hpp"
//#include "../helpers/utils.hpp"
//#include "../singleton.hpp"
//#include "../Hooks.hpp"
//#include "eventlistener.hpp"
//
//
//struct log_data_t
//{
//    std::string msg = "";
//    float time = 0.f;
//};
//
//struct event_data_t
//{
//    Vector pos = Vector(0, 0, 0);
//    bool got_pos = false;
//    bool got_hurt = false;
//    float time = 0.f;
//    int hurt_player = -1;
//    int userid = -1;
//    bool hurt = false;
//    int damage = 0;
//    bool died = false;
//    int hitbox = HITBOX_MAX;
//};
//
//class c_event_logger
//{
//public:
//    void on_fire_event(IGameEvent* event);
//    void on_create_move();
//    void on_draw();
//
//    void set_rbot_data(C_BasePlayer* player, int index, QAngle angle);
//    //void set_rbot_data_backtrack(C_BasePlayer* player, int index, QAngle angle, backtrack data);
//private:
//    int last_rbot_entity = -1;
//    float last_rbot_shot_time = 0.f;
//  //  player_backup_data_t last_shot_pdata;
//    Vector last_rbot_shot_eyepos = Vector(0, 0, 0);
//    QAngle last_rbot_shot_angle = QAngle(0, 0, 0);
//   // c_resolver::resolver_data_t last_rbot_resolver_data;
//
//    enum class miss_type_t : int
//    {
//        none,
//        spread,
//        resolver,
//        custom
//    };
//
//    std::deque< event_data_t > last_events;
//    std::deque< log_data_t > logs;
//    void log_damage(C_BasePlayer* player, int damage, int hitgroup);
//    void draw_beam(Vector startpos, Color color, Vector pos) const;
//    void draw_footstep_beam(C_BasePlayer* player) const;
//
//    template < typename t >
//    static std::string to_string_with_precision(const t a_value, const int n = 6)
//    {
//        std::ostringstream out;
//        out.precision(n);
//        out << std::fixed << a_value;
//        return out.str();
//    }
//};
