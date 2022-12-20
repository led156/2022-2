from torch.optim import SGD #Default optimizer. you may use other optimizers
from torch.utils.data import DataLoader
import argparse
import torch
import torch.nn as nn
import gym
from utils import get_explore_rate, select_action
from model import Q_net

####################################################
import datetime
from torch.utils.tensorboard import SummaryWriter
import random

replay_memory_fail = []
replay_memory_success = []

def _insert_record(memorys, rec, memory_size = -1):
    for i in range(len(memorys)):
        if (rec[0]==memorys[i][0]).all() and (rec[2], memorys[i][2]).all() and rec[1] == memorys[i][1] and rec[3:] == memorys[i][3:]:
            print(f"{rec}=={memorys[i]}")
            return None
    if len(memorys) < memory_size or memory_size == -1:
        memorys.append(rec)
    else:
        memorys[random.randint(0, memory_size-1)] = rec
        
# rec = (state_0, action, state, reward, done)
def insert_record(rec, args):
    done = rec[-1] # done
    if done: # success
        _insert_record(replay_memory_success, rec, args.max_memory/2)
    else: # fail
        _insert_record(replay_memory_fail, rec, args.max_memory/2)
    return replay_memory_success + replay_memory_fail
####################################################


def simulate(model, args): #model: the neural network
    #optimizer and loss for the neural network
    optimizer = SGD(model.parameters(), lr = 0.01)
    criterion = nn.MSELoss()
    ## Instantiating the learning related parameters
    explore_rate = get_explore_rate(0, args.decay_constant, args.min_explore_rate)

    dt = datetime.datetime.now() ## 기록 title 설정
    dt_str = dt.strftime("%Y-%m-%d+%Hh%Mm%Ss")
    opt_str = f"batch_size{args.batchsize}+memory_size{args.max_memory}+discount_factor{args.discount_factor}+decay_constant{args.decay_constant}"
    writer = SummaryWriter(f"runs/{opt_str}_{dt_str}")


    memory = list()
    num_streaks = 0
    for episode in range(args.num_episodes):
        # Reset the environment
        state_0 = env.reset()
        loss = 0
        for t in range(args.max_timestep):
            # env.render()#you may want to comment this line out, to run code silently without rendering
            
            # Selecting an action. the action MUST be choosed from the neural network's output.
            with torch.no_grad():
                actiontable = model(torch.Tensor(state_0).unsqueeze(0))
                action = select_action(actiontable.squeeze(0), explore_rate, env)

            # Execute the action then collect outputs
            state, reward, done, _ = env.step(action)

            #Update the memory
            ####################################################
            # TODO:Implement your memory (SAVE, insert_record)
            new_tuple = (state_0, action, state, reward, done)
            
            memory = insert_record(new_tuple, args)
            ####################################################
            
            
            # Update the Q-net parameters
            loss = replay(model, memory, args, criterion, optimizer, iteration = 1)
            
            state_0 = state

            #done: the cart failed to maintain balance
            if done == True:
                break

        # Update parameters
        explore_rate = get_explore_rate(episode, args.decay_constant, args.min_explore_rate)
        if loss != -1 :
            writer.add_scalar("Step(reward)", t, episode)
            writer.add_scalar("Loss", loss, episode)
        print("Episode %d finished after %f time steps. Streak: %d" % (episode, t, num_streaks))

        #The episode is considered as a success if timestep >SOLVED_TIMESTEP 
        if (t >= args.solved_timestep):
            num_streaks += 1
        else:
            num_streaks = 0
            
        #  when the agent 'solves' the environment: steak over 120 times consecutively
        if num_streaks > args.streak_to_end:
            print("The Environment is solved")
            torch.save(model.state_dict(), 'modelparam.pt')
            break

    env.close()#closes window

def replay(model, memory, args, criterion, optimizer, iteration = 1):
    if len(memory) < args.batchsize:
        return -1
    d_loader = DataLoader(memory, args.batchsize ,shuffle = True, drop_last= True)
    loss_i = 0
    
    for i, batch in enumerate(d_loader):
        
        ####################################################
        # TODO: Implement your replay function.
        if (i >= iteration):
            break
        
        optimizer.zero_grad()
        
        state, action, next_state, reward, done = batch
        
        

        # Set target value
        next_state_r_t = next_state # torch.Size([64, 4])
        with torch.no_grad():
            q_next = model(next_state_r_t)
            y_t = reward + (args.discount_factor * q_next.max(dim=-1)[0]) * (1-done.long()) # torch.Size([64])
        
        # Make a prediction
        state_r_t = state # torch.Size([64, 4])
        #with torch.no_grad():
        q_hat = model(state_r_t) # torch.Size([64, 2])
        q_hat = q_hat.gather(1, action.unsqueeze(axis=-1)).squeeze(dim=1)
        #y = reward + gamma * q_hat.max(dim=-1)[0]
        
        # update
        loss = criterion(q_hat.to(torch.float32), y_t.to(torch.float32))
        loss.backward()
        optimizer.step()

        loss_i += loss.item()
        ####################################################
    return loss_i


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Train the model!')
    parser.add_argument('--num_episodes', type = int, default= 10000)
    parser.add_argument('--max_timestep', type = int, default= 250)
    parser.add_argument('--solved_timestep', type = int, default= 199)
    parser.add_argument('--streak_to_end', type = int, default= 120)
    parser.add_argument('--batchsize', type = int, default= 128)
    parser.add_argument('--min_explore_rate', type = float, default= 0.01)
    parser.add_argument('--discount_factor', type = float, default= 0.69)
    parser.add_argument('--decay_constant', type = int, default= 25)
    parser.add_argument('--max_memory', type = int, default = 2000)
    train_args = parser.parse_args()

    env = gym.make('CartPole-v1')
    qnet = Q_net()
    simulate(qnet, train_args)