# search.py
# ---------
# Licensing Information:  You are free to use or extend these projects for
# educational purposes provided that (1) you do not distribute or publish
# solutions, (2) you retain this notice, and (3) you provide clear
# attribution to UC Berkeley, including a link to http://ai.berkeley.edu.
# 
# Attribution Information: The Pacman AI projects were developed at UC Berkeley.
# The core projects and autograders were primarily created by John DeNero
# (denero@cs.berkeley.edu) and Dan Klein (klein@cs.berkeley.edu).
# Student side autograding was added by Brad Miller, Nick Hay, and
# Pieter Abbeel (pabbeel@cs.berkeley.edu).


"""
In search.py, you will implement generic search algorithms which are called by
Pacman agents (in searchAgents.py).
"""

import util
from game import Directions
from typing import List

class SearchProblem:
    """
    This class outlines the structure of a search problem, but doesn't implement
    any of the methods (in object-oriented terminology: an abstract class).

    You do not need to change anything in this class, ever.
    """

    def getStartState(self):
        """
        Returns the start state for the search problem.
        """
        util.raiseNotDefined()

    def isGoalState(self, state):
        """
          state: Search state

        Returns True if and only if the state is a valid goal state.
        """
        util.raiseNotDefined()

    def getSuccessors(self, state):
        """
          state: Search state

        For a given state, this should return a list of triples, (successor,
        action, stepCost), where 'successor' is a successor to the current
        state, 'action' is the action required to get there, and 'stepCost' is
        the incremental cost of expanding to that successor.
        """
        util.raiseNotDefined()

    def getCostOfActions(self, actions):
        """
         actions: A list of actions to take

        This method returns the total cost of a particular sequence of actions.
        The sequence must be composed of legal moves.
        """
        util.raiseNotDefined()




def tinyMazeSearch(problem: SearchProblem) -> List[Directions]:
    """
    Returns a sequence of moves that solves tinyMaze.  For any other maze, the
    sequence of moves will be incorrect, so only use this for tinyMaze.
    """
    s = Directions.SOUTH
    w = Directions.WEST
    return  [s, s, w, s, w, w, s, w]

def depthFirstSearch(problem: SearchProblem) -> List[Directions]:
    """
    Search the deepest nodes in the search tree first.

    Your search algorithm needs to return a list of actions that reaches the
    goal. Make sure to implement a graph search algorithm.

    To get started, you might want to try some of these simple commands to
    understand the search problem that is being passed in:

    print("Start:", problem.getStartState())
    print("Is the start a goal?", problem.isGoalState(problem.getStartState()))
    print("Start's successors:", problem.getSuccessors(problem.getStartState()))
    """
    "*** YOUR CODE HERE ***"
    visited = {problem.getStartState()}
    state_stack = util.Stack()
    state_stack.push([
        problem.getStartState(),
        [],
    ])
    while(not state_stack.isEmpty()):
        state, path = state_stack.pop()
        if(problem.isGoalState(state)):
            return path
        

        for Successor in problem.getSuccessors(state):
            if(Successor[0] not in visited):
                next_path = path.copy()
                next_path.append(Successor[1])
                state_stack.push([Successor[0], next_path])
                visited.add(Successor[0])

    print("[DFS] No Solution Found")
    return []
    # ------------------------------ Ver 2 ------------------------------
    # path = []
    # visited = set()
    # state_stack = util.Stack()
    # state_stack.push([
    #     problem.getStartState(),                            # Current State
    #     problem.getSuccessors(problem.getStartState()),     # Successors (state, action, ?)
    #     None,                                               # Prev State
    # ])
    # while(not state_stack.isEmpty()):
    #     state, successors, prev_state = state_stack.pop()
    #     successors = list(filter(lambda x: x[0] not in visited, successors))    # remove the state that is already visited
    #     visited.add(state)
    #     if(problem.isGoalState(state)):
    #         break

    #     if(len(successors)>0):      # goto successors
    #         nextState, action, dummy = successors.pop()
    #         state_stack.push([state, successors, prev_state])
    #         state_stack.push([nextState, problem.getSuccessors(nextState), state])
    #         path.append(action)
    #     else:                       # goback prevState
    #         if(prev_state == None):
    #             print("[DFS] No Solution Found!!")
    #             break
    #         action = list(filter(lambda x:x[0] == prev_state, problem.getSuccessors(state)))[0][1]
    #         path.append(action)
    # return path
    # util.raiseNotDefined()

def breadthFirstSearch(problem: SearchProblem) -> List[Directions]:
    """Search the shallowest nodes in the search tree first."""
    "*** YOUR CODE HERE ***"
    visited = {problem.getStartState()}
    state_queue = util.Queue()
    state_queue.push([
        problem.getStartState(),
        [],
    ])
    while(not state_queue.isEmpty()):
        state, path = state_queue.pop()
        if(problem.isGoalState(state)):
            return path
        

        for Successor in problem.getSuccessors(state):
            if(Successor[0] not in visited):
                next_path = path.copy()
                next_path.append(Successor[1])
                state_queue.push([Successor[0], next_path])
                visited.add(Successor[0])

    print("[BFS] No Solution Found")
    return []
    # util.raiseNotDefined()

def uniformCostSearch(problem: SearchProblem) -> List[Directions]:
    """Search the node of least total cost first."""
    "*** YOUR CODE HERE ***"
    visited = set()
    state_dict = {problem.getStartState() : [
        [],                         # path
        0                           # total cost
    ]}
    state_pq = util.PriorityQueue()
    state_pq.push(problem.getStartState(),0)
    while(not state_pq.isEmpty()):
        state = state_pq.pop()
        visited.add(state)
        if(problem.isGoalState(state)):
            return state_dict[state][0]
        
        for successor in problem.getSuccessors(state):
            next_state, action, dummy = successor
            if(next_state in visited):
                continue
            
            priority = state_dict[state][1]+1
            if((next_state not in state_dict.keys()) or (priority < state_dict[next_state][1])):
                path = state_dict[state][0].copy()
                path.append(action)
                state_dict[next_state] = [path, priority]
                state_pq.update(next_state, priority)

    print("[UCS] Solution not found")
    return []
    # util.raiseNotDefined()

def nullHeuristic(state, problem=None) -> float:
    """
    A heuristic function estimates the cost from the current state to the nearest
    goal in the provided SearchProblem.  This heuristic is trivial.
    """
    return 0

def aStarSearch(problem: SearchProblem, heuristic=nullHeuristic) -> List[Directions]:
    """Search the node that has the lowest combined cost and heuristic first."""
    "*** YOUR CODE HERE ***"
    visited = set()
    state_dict = {problem.getStartState():[
        [],                                             # path
        heuristic(problem.getStartState(), problem),    # priority in pq
    ]}
    state_pq = util.PriorityQueue()
    state_pq.push(problem.getStartState() ,heuristic(problem.getStartState(), problem))
    while(not state_pq.isEmpty()):
        state = state_pq.pop()
        visited.add(state)
        if(problem.isGoalState(state)):
            return state_dict[state][0]
        
        for successor in problem.getSuccessors(state):
            next_state, action, dummy = successor
            if(next_state in visited):
                continue
            
            # f(n) = h(n) + g(n)
            priority = heuristic(next_state, problem) + len(state_dict[state][0])
            if((next_state not in state_dict.keys()) or (priority < state_dict[next_state][1])):
                path = state_dict[state][0].copy()
                path.append(action)
                state_dict[next_state] = [path, priority]
                state_pq.update(next_state, priority)

    # util.raiseNotDefined()

# Abbreviations
bfs = breadthFirstSearch
dfs = depthFirstSearch
astar = aStarSearch
ucs = uniformCostSearch
