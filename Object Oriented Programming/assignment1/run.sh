# python3 pacman.py --layout testMaze --pacman GoWestAgent

python3 pacman.py --layout tinyMaze --pacman SearchAgent --agentArgs fn=uniformCostSearch
python3 pacman.py --layout smallMaze --pacman SearchAgent --agentArgs fn=uniformCostSearch
python3 pacman.py --layout mediumMaze --pacman SearchAgent --agentArgs fn=uniformCostSearch
python3 pacman.py --layout bigMaze --pacman SearchAgent --agentArgs fn=uniformCostSearch
echo -------------------------------------------------------------------------------------
python3 pacman.py --layout testMaze --pacman GoWestAgent
python3 pacman.py --layout tinyMaze --pacman SearchAgent --agentArgs fn=breadthFirstSearch
python3 pacman.py --layout smallMaze --pacman SearchAgent --agentArgs fn=breadthFirstSearch
python3 pacman.py --layout mediumMaze --pacman SearchAgent --agentArgs fn=breadthFirstSearch
python3 pacman.py --layout bigMaze --pacman SearchAgent --agentArgs fn=breadthFirstSearch
echo -------------------------------------------------------------------------------------
python3 pacman.py --layout tinyMaze --pacman SearchAgent
python3 pacman.py --layout smallMaze --pacman SearchAgent
python3 pacman.py --layout mediumMaze --pacman SearchAgent
python3 pacman.py --layout bigMaze --pacman SearchAgent