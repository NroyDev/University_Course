# python3 pacman.py --layout testMaze --pacman GoWestAgent


agentArgs='fn=aStarSearch,heuristic=cornersHeuristic,prob=CornersProblem'
python3 pacman.py --layout tinyCorners --pacman SearchAgent --agentArgs "$agentArgs"
python3 pacman.py --layout mediumCorners --pacman SearchAgent --agentArgs "$agentArgs"
python3 pacman.py --layout bigCorners --pacman SearchAgent --agentArgs "$agentArgs"
echo -------------------------------------------------------------------------------------
agentArgs='fn=uniformCostSearch,prob=CornersProblem'
python3 pacman.py --layout tinyCorners --pacman SearchAgent --agentArgs "$agentArgs"
python3 pacman.py --layout mediumCorners --pacman SearchAgent --agentArgs "$agentArgs"
python3 pacman.py --layout bigCorners --pacman SearchAgent --agentArgs "$agentArgs"
echo -------------------------------------------------------------------------------------
agentArgs='fn=breadthFirstSearch,prob=CornersProblem'
python3 pacman.py --layout tinyCorners --pacman SearchAgent --agentArgs "$agentArgs"
python3 pacman.py --layout mediumCorners --pacman SearchAgent --agentArgs "$agentArgs"
python3 pacman.py --layout bigCorners --pacman SearchAgent --agentArgs "$agentArgs"
echo -------------------------------------------------------------------------------------
agentArgs='fn=depthFirstSearch,prob=CornersProblem'
python3 pacman.py --layout tinyCorners --pacman SearchAgent --agentArgs "$agentArgs"
python3 pacman.py --layout mediumCorners --pacman SearchAgent --agentArgs "$agentArgs"
python3 pacman.py --layout bigCorners --pacman SearchAgent --agentArgs "$agentArgs"
echo -------------------------------------------------------------------------------------

# agentArgs='fn=aStarSearch,heuristic=manhattanHeuristic'
# python3 pacman.py --layout tinyMaze --pacman SearchAgent --agentArgs "$agentArgs"
# python3 pacman.py --layout smallMaze --pacman SearchAgent --agentArgs "$agentArgs"
# python3 pacman.py --layout mediumMaze --pacman SearchAgent --agentArgs "$agentArgs"
# python3 pacman.py --layout bigMaze --pacman SearchAgent --agentArgs "$agentArgs"
# echo -------------------------------------------------------------------------------------
# agentArgs='fn=uniformCostSearch'
# python3 pacman.py --layout tinyMaze --pacman SearchAgent --agentArgs "$agentArgs"
# python3 pacman.py --layout smallMaze --pacman SearchAgent --agentArgs "$agentArgs"
# python3 pacman.py --layout mediumMaze --pacman SearchAgent --agentArgs "$agentArgs"
# python3 pacman.py --layout bigMaze --pacman SearchAgent --agentArgs "$agentArgs"
# echo ------------------------------------------------------------------------------------
# agentArgs='fn=breadthFirstSearch'
# python3 pacman.py --layout tinyMaze --pacman SearchAgent --agentArgs "$agentArgs"
# python3 pacman.py --layout smallMaze --pacman SearchAgent --agentArgs "$agentArgs"
# python3 pacman.py --layout mediumMaze --pacman SearchAgent --agentArgs "$agentArgs"
# python3 pacman.py --layout bigMaze --pacman SearchAgent --agentArgs "$agentArgs"
# echo -------------------------------------------------------------------------------------
# agentArgs='fn=depthFirstSearch'
# python3 pacman.py --layout tinyMaze --pacman SearchAgent --agentArgs "$agentArgs"
# python3 pacman.py --layout smallMaze --pacman SearchAgent --agentArgs "$agentArgs"
# python3 pacman.py --layout mediumMaze --pacman SearchAgent --agentArgs "$agentArgs"
# python3 pacman.py --layout bigMaze --pacman SearchAgent --agentArgs "$agentArgs"