# python3 pacman.py --layout testMaze --pacman GoWestAgent
# clear

# test
# agentArgs='fn=aStarSearch,heuristic=foodHeuristic,prob=FoodSearchProblem'
# python3 pacman.py --layout tinySafeSearch --pacman SearchAgent --agentArgs "$agentArgs"
# python3 pacman.py --layout smallSafeSearch --pacman SearchAgent --agentArgs "$agentArgs"
# python3 pacman.py --layout mediumSafeSearch --pacman SearchAgent --agentArgs "$agentArgs"
# python3 pacman.py --layout bigSafeSearch --pacman SearchAgent --agentArgs "$agentArgs"

# AnyFoodSearchProblem
agentArgs='fn=breadthFirstSearch,prob=AnyFoodSearchProblem'
python3 pacman.py --layout tinySafeSearch --pacman ClosestDotSearchAgent  --agentArgs "$agentArgs"
python3 pacman.py --layout mediumSafeSearch --pacman ClosestDotSearchAgent  --agentArgs "$agentArgs"
python3 pacman.py --layout bigSafeSearch --pacman ClosestDotSearchAgent  --agentArgs "$agentArgs"
echo -------------------------------------------------------------------------------------

# FoodSearchProblem
agentArgs='fn=aStarSearch,heuristic=foodHeuristic,prob=FoodSearchProblem'
python3 pacman.py --layout tinySafeSearch --pacman SearchAgent --agentArgs "$agentArgs"
python3 pacman.py --layout smallSafeSearch --pacman SearchAgent --agentArgs "$agentArgs"
python3 pacman.py --layout mediumSafeSearch --pacman SearchAgent --agentArgs "$agentArgs"
echo -------------------------------------------------------------------------------------
agentArgs='fn=uniformCostSearch,prob=FoodSearchProblem'
python3 pacman.py --layout trickySearch --pacman SearchAgent --agentArgs "$agentArgs"
python3 pacman.py --layout trickySearch --pacman SearchAgent --agentArgs "$agentArgs"
python3 pacman.py --layout trickySearch --pacman SearchAgent --agentArgs "$agentArgs"
echo -------------------------------------------------------------------------------------
agentArgs='fn=breadthFirstSearch,prob=FoodSearchProblem'
python3 pacman.py --layout tinySafeSearch --pacman SearchAgent --agentArgs "$agentArgs"
python3 pacman.py --layout smallSafeSearch --pacman SearchAgent --agentArgs "$agentArgs"
python3 pacman.py --layout mediumSafeSearch --pacman SearchAgent --agentArgs "$agentArgs"
echo -------------------------------------------------------------------------------------
agentArgs='fn=depthFirstSearch,prob=FoodSearchProblem'
python3 pacman.py --layout tinySafeSearch --pacman SearchAgent --agentArgs "$agentArgs"
python3 pacman.py --layout smallSafeSearch --pacman SearchAgent --agentArgs "$agentArgs"
python3 pacman.py --layout mediumSafeSearch --pacman SearchAgent --agentArgs "$agentArgs"

# CornersProblem
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

# PositionSearchProblem
agentArgs='fn=aStarSearch,heuristic=manhattanHeuristic'
python3 pacman.py --layout tinyMaze --pacman SearchAgent --agentArgs "$agentArgs"
python3 pacman.py --layout smallMaze --pacman SearchAgent --agentArgs "$agentArgs"
python3 pacman.py --layout mediumMaze --pacman SearchAgent --agentArgs "$agentArgs"
python3 pacman.py --layout bigMaze --pacman SearchAgent --agentArgs "$agentArgs"
echo -------------------------------------------------------------------------------------
agentArgs='fn=uniformCostSearch'
python3 pacman.py --layout tinyMaze --pacman SearchAgent --agentArgs "$agentArgs"
python3 pacman.py --layout smallMaze --pacman SearchAgent --agentArgs "$agentArgs"
python3 pacman.py --layout mediumMaze --pacman SearchAgent --agentArgs "$agentArgs"
python3 pacman.py --layout bigMaze --pacman SearchAgent --agentArgs "$agentArgs"
echo ------------------------------------------------------------------------------------
agentArgs='fn=breadthFirstSearch'
python3 pacman.py --layout tinyMaze --pacman SearchAgent --agentArgs "$agentArgs"
python3 pacman.py --layout smallMaze --pacman SearchAgent --agentArgs "$agentArgs"
python3 pacman.py --layout mediumMaze --pacman SearchAgent --agentArgs "$agentArgs"
python3 pacman.py --layout bigMaze --pacman SearchAgent --agentArgs "$agentArgs"
echo -------------------------------------------------------------------------------------
agentArgs='fn=depthFirstSearch'
python3 pacman.py --layout tinyMaze --pacman SearchAgent --agentArgs "$agentArgs"
python3 pacman.py --layout smallMaze --pacman SearchAgent --agentArgs "$agentArgs"
python3 pacman.py --layout mediumMaze --pacman SearchAgent --agentArgs "$agentArgs"
python3 pacman.py --layout bigMaze --pacman SearchAgent --agentArgs "$agentArgs"