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
    last_state = problem.getStartState()
    state_dict = {problem.getStartState(): None}     # key: state, value: (prev_state, action_tostate)
    state_stack = util.Stack()
    state_stack.push(problem.getStartState())        # state
    while(not state_stack.isEmpty()):
        last_state = state = state_stack.pop()
        if(problem.isGoalState(state)):
            break

        successors = problem.getSuccessors(state)
        for successor in successors:
            next_state, action, cost = successor
            if(next_state not in visited):
                visited.add(next_state)
                state_dict[next_state] = (state, action)
                state_stack.push(next_state)
    
    path = []
    while(state_dict[last_state] != None):
        path.append(state_dict[last_state][1])
        last_state = state_dict[last_state][0]

    return path[::-1]

def breadthFirstSearch(problem: SearchProblem) -> List[Directions]:
    """Search the shallowest nodes in the search tree first."""
    "*** YOUR CODE HERE ***"
    visited = {problem.getStartState()}
    last_state = problem.getStartState()
    state_dict = {problem.getStartState(): None}     # key: state, value: (prev_state, action_tostate)
    state_queue = util.Queue()
    state_queue.push(problem.getStartState())        # state
    while(not state_queue.isEmpty()):
        last_state = state = state_queue.pop()
        if(problem.isGoalState(state)):
            break

        successors = problem.getSuccessors(state)
        for successor in successors:
            next_state, action, cost = successor
            if(next_state not in visited):
                visited.add(next_state)
                state_dict[next_state] = (state, action)
                state_queue.push(next_state)
    
    path = []
    while(state_dict[last_state] != None):
        path.append(state_dict[last_state][1])
        last_state = state_dict[last_state][0]

    return path[::-1]

def uniformCostSearch(problem: SearchProblem) -> List[Directions]:
    """Search the node of least total cost first."""
    "*** YOUR CODE HERE ***"
    visited = set()
    last_state = problem.getStartState()
    state_dict = {problem.getStartState(): (None, 0)}    # key: state, value: ((prev_state, action_tostate), cost)
    state_pq = util.PriorityQueue()
    state_pq.push(problem.getStartState(), 0)
    while(not state_pq.isEmpty()):
        last_state = state = state_pq.pop()
        visited.add(state)
        if(problem.isGoalState(state)):
            break

        successors = problem.getSuccessors(state)
        for successor in successors:
            next_state, action, cost = successor
            if(next_state in visited):
                continue
            new_priority = state_dict[state][1] + problem.getCostOfActions([action])
            if((next_state not in state_dict.keys()) or (new_priority < state_dict[next_state][1])):
                state_dict[next_state] = ((state, action), new_priority)
                state_pq.update(next_state, new_priority)

    path = []
    while(state_dict[last_state][0] != None):
        path.append(state_dict[last_state][0][1])
        last_state = state_dict[last_state][0][0]

    return path[::-1]

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
    last_state = problem.getStartState()
    state_dict = {problem.getStartState(): (None, 0, 0)}    # key: state, value: ((prev_state, action_tostate), cost, dist_start)
    state_pq = util.PriorityQueue()
    state_pq.push(problem.getStartState(), 0)
    while(not state_pq.isEmpty()):
        last_state = state = state_pq.pop()
        visited.add(state)
        if(problem.isGoalState(state)):
            break

        successors = problem.getSuccessors(state)
        for successor in successors:
            next_state, action, cost = successor
            if(next_state in visited):
                continue

            # f(n) = h(n) + g(n)
            new_priority = heuristic(next_state, problem) + (state_dict[state][2]+1)
            if((next_state not in state_dict.keys()) or (new_priority < state_dict[next_state][1])):
                state_dict[next_state] = ((state, action), new_priority, state_dict[state][2]+1)
                state_pq.update(next_state, new_priority)

    path = []
    while(state_dict[last_state][0] != None):
        path.append(state_dict[last_state][0][1])
        last_state = state_dict[last_state][0][0]

    return path[::-1]

# Abbreviations
bfs = breadthFirstSearch
dfs = depthFirstSearch
astar = aStarSearch
ucs = uniformCostSearch
