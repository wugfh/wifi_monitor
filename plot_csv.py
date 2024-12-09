import csv
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np

slot = 9

def csv_time(ack_path, data_path, suffix):
    ack = pd.read_csv(ack_path)
    data = pd.read_csv(data_path)
    
    data_sn = data.loc[:,'frame_sn']
    data_time = data.loc[:,"time"]
    data_seq = data.loc[:,"subseq"]
    ack_sn = ack.loc[:,'frame_sn']
    ack_time = ack.loc[:,'time']
    i = 0
    j = 0
    time = []
    sn = []
    name = []
    seq = []
    while(i < len(ack_time) or j < len(data_time)):
        if(j < len(data_time)):
            while(i < len(ack_time) and ack_time[i] <= data_time[j]):
                time.append(ack_time[i])
                sn.append(ack_sn[i])
                name.append("BA")
                i = i+1
                seq.append(-1)
        else:
            while(i < len(ack_time)):
                time.append(ack_time[i])
                sn.append(ack_sn[i])
                name.append("BA")
                seq.append(-1)
                i = i+1
        if(i < len(ack_time)):
            while(j < len(data_time)and ack_time[i] > data_time[j]):
                time.append(data_time[j])
                sn.append(data_sn[j])
                name.append("DATA")
                seq.append(data_seq[j])
                j = j+1
        else:
            while(j < len(data_time)):
                time.append(data_time[j])
                sn.append(data_sn[j])
                name.append("DATA")
                seq.append(data_seq[j])
                j = j+1
        # print("i:",i,"j:",j)
    frame = pd.DataFrame({"sn":sn, "time":time, "type":name, "seq":seq})
    frame.to_csv("mon_csv/time_analyse_"+suffix+".csv", index=False)
    
def calculate_delay(time_analyse, suffix):
    out_seq = []
    delay = []
    data = pd.read_csv(time_analyse)
    type = data.loc[:,"type"]
    time = data.loc[:,"time"]
    seq = data.loc[:,"seq"]
    sn = data.loc[:,"sn"]
    data_stack = set()
    sn2pos = {}
    remove_cnt = 0
    latest_seq = 0
    for i in range(len(time)):
        if(type[i] == "DATA"):
            if(sn[i] in sn2pos.keys()):
                pos = sn2pos[sn[i]]
                data_stack.remove(pos)
                data_stack.add(i)
                remove_cnt = remove_cnt+1
                sn2pos[sn[i]] = i
                latest_seq = seq[i]
            else: 
                data_stack.add(i)
                sn2pos[sn[i]] = i
                latest_seq = seq[i]
        elif(type[i] == "BA"):
            pre = -1
            sn_i = sn[i]
            pop_array = []
            for j in data_stack:
                sn_j = sn[j]
                seq_j = seq[j]
                time_j = time[j]
                # print("sn_i:", sn_i, "sn_j:",sn_j)
                if(sn_i > 4000 and sn_j < 100):sn_j = sn_j + 4096
                if(sn_j == sn_i+63): 
                    if(abs(seq_j-latest_seq) > 4096):
                        pop_array.append(j)
                        remove_cnt = remove_cnt+1
                        continue
                    out_seq.append(seq_j)
                    delay.append((time[i]-time_j))
                    pop_array.append(j)
            for x in pop_array:
                data_stack.remove(x)
                sn2pos.pop(sn[x])
    frame = pd.DataFrame({"seq":out_seq, "time":delay})
    frame.to_csv("mon_csv/interval_"+suffix+".csv", index=False)
    print("pack no ack captured:", len(data_stack)+remove_cnt)

def plot_delay(interval, mode, pos, total):
    delay = pd.read_csv(interval)
    
    delay_time = delay.loc[:,"time"]
    q1 = np.percentile(delay_time, 1)
    q3 = np.percentile(delay_time, 99)
    effect_t = []
    for t in delay_time:
        if(t > q1 and t < q3):
            effect_t.append(t)
    
    x = np.linspace(0, len(effect_t), len(effect_t))
    plt.subplot(1,total, pos)
    plt.scatter(x, effect_t, marker=".", label=mode+"chan")
    plt.legend()
    plt.xlabel("sequence")
    plt.ylabel("delay/us")
    plt.grid() 
    print(np.mean(effect_t), "     the percent of effective delay value ", len(effect_t)/len(delay_time))

def resolve_data(data_capture, data_start, suffix):
    capture = pd.read_csv(data_capture)
    start = pd.read_csv(data_start)
    capture.dropna(axis=0, how="any")
    start.dropna(axis=0, how="any")
    start_seq = start.loc[:,"seq"]
    start_time = start.loc[:,"time"]
    cp_seq = capture.loc[:,"seq"]
    cp_sn = capture.loc[:,"frame_sn"]
    j = 0
    time = []
    sn = []
    seq = []
    for i in range(len(cp_seq)):
        if(cp_seq[i] < start_seq[j]):continue
        elif(start_seq[j] == cp_seq[i]):
            time.append(start_time[j])
            sn.append(cp_sn[i])
            seq.append(cp_seq[i])
            j = j+1
        else:
            while(start_seq[j] < cp_seq[i]):j = j+1
            if(start_seq[j] == cp_seq[i]):
                time.append(start_time[j])
                sn.append(cp_sn[i])
                seq.append(cp_seq[i])
                j = j+1
    frame = pd.DataFrame({"frame_sn":sn, "time":time, "subseq":seq})
    frame.to_csv("send_csv/csv/data_mon_"+suffix+".csv", index=False)
    
def plot_jitter(ack_path, pos, total, mode):
    ack = pd.read_csv(ack_path)
    ack_time = ack.loc[:,'time']
    jitter = ack_time.diff()
    x = np.linspace(0, len(jitter), len(jitter))
    plt.subplot(1,total,pos)
    plt.scatter(x, jitter, marker=".", label="channel "+mode)
    plt.legend()
    plt.xlabel("sequence")
    plt.ylabel("delay/us")
    plt.grid()
    print(np.mean(jitter))

print("165:")
resolve_data("mon_csv/csv/data_mon_165.csv", "send_csv/csv/send_time.csv", "165") 
# csv_time("mon_csv/csv/ba_mon_165.csv", "mon_csv/csv/data_mon_165.csv", "165")
# calculate_delay("mon_csv/time_analyse_165.csv", "165")  
csv_time("send_csv/csv/ba_mon_165.csv", "send_csv/csv/data_mon_165.csv", "165")
calculate_delay("mon_csv/time_analyse_165.csv", "165")
plot_delay("mon_csv/interval_165.csv","165", 1, 2)

resolve_data("mon_csv/csv/data_mon_48.csv", "send_csv/csv/send_time.csv", "48")
# csv_time("mon_csv/csv/ba_mon_40.csv", "mon_csv/csv/data_mon_40.csv", "40")
# calculate_delay("mon_csv/time_analyse_40.csv", "40")
csv_time("send_csv/csv/ba_mon_48.csv", "send_csv/csv/data_mon_48.csv", "48")
calculate_delay("mon_csv/time_analyse_48.csv", "48")
plot_delay("mon_csv/interval_48.csv", "48", 2, 2)
plt.show()

# plot_jitter("send_csv/csv/ba_mon_48.csv", 1, 2, "48")
# plot_jitter("send_csv/csv/ba_mon_165.csv", 2, 2, "165")
# plt.show()

