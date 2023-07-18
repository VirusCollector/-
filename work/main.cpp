#include <iostream>
#include <vector>
#include <algorithm>
#include <assert.h>
using namespace std;
const int ShipNum = 20;
const int PortNum = 20;
const double PI = 3.14159267;
const double PortPara = 0.12;
const double ShipPara = 0.12;
const double InitTemp = 12800000;
const double beta = 400000;
const int Step = 9;
const double alpha = 0.99;

struct Ship
{
    int id;
    int EnterTime;
    int Width;
    int Depth;
    int WorkTime;

    Ship(int Id, int time, int width, int depth, int worktime)
    {
        id = Id;
        EnterTime = time;
        Width = width;
        Depth = depth;
        WorkTime = worktime;
    }
};
vector <Ship> ships;

struct Port
{
    int PortId;
    int DepthProvided;
    int width;
};
struct Event
{
    int time;
    int id;
    Event(int t, int s)
    {
        time = t;
        id = s;
    }
    bool operator<(const Event& e) const
    {
        if (time != e.time)
            return time < e.time;
        else
        {
            return ships[id].Width < ships[e.id].Width;
        }
    }
    bool operator>(const Event& e) const
    {
        return time > e.time;
    }
};
struct Info
{
    int PortId;
    int id;
    int EnterTime;
    int ServeTime;
    int WorkTime;
    int WaitTime;
};

vector <Port> ports;
vector <Event> events;
vector <Info> msg;
vector <Info> BestMsg;
vector <Info> BestAnswer;

vector <pair<int, int>> HeuAnswer;
vector <int> PortSheet[160];
vector <pair<int, int>> TempAnswer;
vector <int> PortsAvailable[160];
int PortNextTime[20];
char filepath[30] = { 0 };
int BestTime = 99999999;

double GetDepth(int depth, int time)
{
    return depth + 2 * sin(2 * PI * time / 1440);
}
int check()
{
    memset(PortNextTime, 0, 80);
    msg.clear();
    for (int i = 0; i < ShipNum; i++)
    {
        int id = TempAnswer[i].first;
        int PortId = TempAnswer[i].second;
        Info m;
        m.id = id;
        m.PortId = PortId;
        m.EnterTime = ships[id].EnterTime;
        m.WorkTime = ships[id].WorkTime;
        msg.push_back(m);
    }
    for (int i = 0; i < ShipNum; i++)
    {
        int id = msg[i].id;
        int PortId = msg[i].PortId;
        int now_time = max(PortNextTime[PortId], msg[i].EnterTime);
        int lefttime = now_time + msg[i].WorkTime;
        double tt = GetDepth(ports[PortId].DepthProvided, lefttime);
        if (ships[id].Depth <= tt)//如果离开港口时不搁浅
        {
            msg[i].ServeTime = now_time;
            msg[i].WaitTime = lefttime - msg[i].EnterTime;
            PortNextTime[PortId] = lefttime;
            continue;
        }
        //离开港口时会搁浅，需要调整时间
        double solution = asin((double)(ships[id].Depth - ports[PortId].DepthProvided) / 2) * 720 / PI;
        for (int p = 0;; p += 1440)
        {
            if (solution + p > lefttime)
            {
                lefttime = int(solution + p) + 1;
                msg[i].ServeTime = lefttime - msg[i].WorkTime;
                msg[i].WaitTime = lefttime - msg[i].EnterTime;
                PortNextTime[PortId] = lefttime;
                break;
            }
        }
    }
    int time0 = 0;
    for (int i = 0; i < ShipNum; i++)
    {
        time0 += msg[i].WaitTime;
        bool a = ships[msg[i].id].Depth > GetDepth(ports[msg[i].PortId].DepthProvided, msg[i].EnterTime);
        bool b = ships[msg[i].id].Depth > GetDepth(ports[msg[i].PortId].DepthProvided, msg[i].EnterTime + msg[i].WaitTime);
        if (a || b)
        {
            assert(0);
        }
    }
    return time0;
}
void init()
{
    if (!freopen("./data/ports.txt", "r", stdin))
        assert(0);
    for (int i = 0; i < PortNum; ++i)
    {
        int id, depth, width;
        cin >> id >> width >> depth;
        Port p;
        p.PortId = id - 1;
        p.width = width;
        p.DepthProvided = depth;
        ports.push_back(p);
    }
    fclose(stdin);
    char filename[30] = { 0 };
    sprintf(filename, "./data/ships%d.txt", ShipNum);
    if (!freopen(filename, "r", stdin))
        assert(0);
    for (int i = 0; i < ShipNum; i++)
    {
        int id, WorkTime, EnterTime, width, depth;
        cin >> id >> EnterTime >> WorkTime >> width >> depth;
        Ship s(i, EnterTime, width, depth, WorkTime);
        ships.push_back(s);
        Event e(EnterTime, id - 1);
        events.push_back(e);
    }
    sort(events.begin(), events.end());
    fclose(stdin);
    return;
}
void search()
{
    for (int i = 0; i < ShipNum; ++i)
    {
        Ship s = ships[events[i].id];
        for (int j = 0; j < PortNum; ++j)
        {
            if (s.Width <= ports[j].width)
            {
                if (s.Depth <= GetDepth(ports[j].DepthProvided, s.EnterTime))
                {
                    PortsAvailable[events[i].id].push_back(ports[j].PortId);
                }
            }
        }
    }
    string filename = string(filepath) + "sheet.txt";
    if (!freopen(filename.c_str(), "w", stdout))
        assert(0);
    for (int i = 0; i < ShipNum; ++i)
    {
        Event event = events[i];
        cout << event.id << " " << PortsAvailable[event.id].size() << " ";
        for (auto j : PortsAvailable[event.id])
        {
            cout << j << " ";
        }
        cout << endl;
    }
    fclose(stdout);
    return;
}
void fire()
{
    string filename = string(filepath) + "fire.txt";
    if (!freopen(filename.c_str(), "w", stdout))
        assert(0);
    cout << "-------------------------" << endl;
    int NowTime = 0;
    double t = InitTemp;
    int T = 0;
    while (T < beta)
    {
        t *= 0.8;
        TempAnswer.clear();
        for (int i = 0; i < ShipNum; i++)
        {
            double r = rand() / (RAND_MAX + 1.0);
            if (r < PortPara * (1 - (double)T / beta))
            {
                int nums = PortSheet[HeuAnswer[i].first].size();
                int index = rand() % nums;
                TempAnswer.push_back(make_pair(HeuAnswer[i].first, PortSheet[HeuAnswer[i].first][index]));
            }
            else
            {
                TempAnswer.push_back(make_pair(HeuAnswer[i].first, HeuAnswer[i].second));
            }
        }
        for (int i = 0; i < ShipNum; i++)
        {
            double r = rand() / (RAND_MAX + 1.0);
            if (r < ShipPara * (1 - (double)T / beta))
            {
                pair<int, int> p = TempAnswer[i];
                int index = max((double)1, Step * (1 - T / beta)) + i;
                index %= ShipNum;
                if (index != i)
                {
                    TempAnswer[i].first = TempAnswer[index].first;
                    TempAnswer[i].second = TempAnswer[index].second;
                    TempAnswer[index].first = p.first;
                    TempAnswer[index].second = p.second;
                }
            }
        }
        NowTime = check();
        if (NowTime < BestTime)
        {
            BestMsg = msg;
            BestTime = NowTime;
            cout << BestTime << endl;
            for (int i = 0; i < ShipNum; i++)
            {
                cout << TempAnswer[i].first << " " << TempAnswer[i].second << endl;
            }
            cout << "-------------------------" << endl;
        }
        else
        {
            double r = rand() / (RAND_MAX + 1.0);
            if (r >= exp(-(NowTime - BestTime) / t))
            {
                TempAnswer.clear();
                for (auto u : HeuAnswer)
                {
                    TempAnswer.push_back(u);
                }
            }
        }
        ++T;
    }
    fclose(stdout);
    filename = string(filepath) + "final.txt";
    freopen(filename.c_str(), "w", stdout);
    memset(PortNextTime, 0, 80);
    int totaltime = 0;
    for (int i = 0; i < ShipNum; ++i)
    {
        Info tempmsg = BestMsg[i];
        totaltime += tempmsg.WaitTime;
        cout << tempmsg.id << " " << tempmsg.PortId << " " << tempmsg.EnterTime << " " << tempmsg.ServeTime << " " << tempmsg.EnterTime + tempmsg.WaitTime << endl;
    }
    cout << "final time: " << totaltime << endl;
    fclose(stdout);
    return;
}
void heuristic()
{
    int PortUsed[20] = { 0 };
    int NextTime[20] = { 0 };
    string filename = string(filepath) + "sheet.txt";
    if (!freopen(filename.c_str(), "r", stdin))
        assert(0);
    int shipnum[160] = { 0 }; 
    for (int i = 0; i < ShipNum; ++i)
    {
        int ship, port, num;
        cin >> ship >> num;
        shipnum[i] = ship;
        for (int j = 0; j < num; ++j)
        {
            cin >> port;
            PortSheet[ship].push_back(port);
        }
    }
    fclose(stdin);
    filename = string(filepath) + "heuristic.txt";
    if (!freopen(filename.c_str(), "w", stdout))
        assert(0);
    for (int s = 0; s < 50000; ++s)
    {
        srand(s);
        TempAnswer.clear();
        memset(PortUsed, 0, 80);
        memset(NextTime, 0, 80);
        for (int i = 0; i < ShipNum; i++)
        {
            int ship = shipnum[i], minused = 0x7fffffff, mintime = 0x7fffffff;
            vector <int> choices, choices1, res;
            vector <int> ports = PortSheet[ship];
            int wait = 0;
            for (auto t : ports)
            {
                wait = max(NextTime[t], ships[ship].EnterTime) + ships[ship].WorkTime - ships[ship].EnterTime;
                if (wait < mintime)
                    mintime = wait;
            }
            for (auto t : ports)
            {
                wait = max(NextTime[t], ships[ship].EnterTime) + ships[ship].WorkTime - ships[ship].EnterTime;
                if (wait > mintime)
                    continue;
                choices1.push_back(t);
            }
            for (auto t : choices1)
            {
                if (PortUsed[t] < minused)
                    minused = PortUsed[t];
            }
            for (auto t : choices1)
            {
                if (PortUsed[t] > minused)
                    continue;
                res.push_back(t);
            }         
            int r = rand() % res.size();
            TempAnswer.push_back(pair<int, int>(ship, res[r]));
            ++PortUsed[res[r]];
            NextTime[res[r]] = max(NextTime[res[r]], ships[ship].EnterTime) + ships[ship].WorkTime;      
        }
        int thisans = check();
        if (thisans < BestTime)
        {
            HeuAnswer.clear();
            BestTime = thisans;
            BestMsg = msg;
            cout << "------------" << endl;
            cout << "time: " << thisans << endl;
            for (int i = 0; i < ShipNum; ++i)
            {
                cout << TempAnswer[i].first << " " << TempAnswer[i].second << endl;
            }
            cout << "finish at " << s << endl;
            for (int i = 0; i < ShipNum; ++i)
                HeuAnswer.push_back(TempAnswer[i]);
        }            
    }    
    fclose(stdout);
}
int main()
{   
    sprintf(filepath, "./result/%d/%d-", ShipNum, ShipNum);
    init();
    search();
    heuristic();
    fire();
    return 0;
}
