/**************
 **  Start ZeroSolver 
 **		Written by: Scott Lindeneau
 **  Requierments: gDatabase intialized w/ all positions set to undecided
 **  Benifits: Almost no memory overhead for the solver. 
 **			  9 POSITION's, 2 int's, 1 VALUE, 2 MOVELIST pointers
 **				32bit machines: around 60 bytes
 **				64bit machines: around 96 bytes
 **					(this depends on the size of VALUE)
 **			  plus whatever the movelist adds, varies per parent (possibly big)
 **
 **  Calls: StoreValueOfPosition, GetValueOfPosition
 **		   MarkAsVisited, Visited
 **         GenerateMoves, DoMove, FreeMoveList
 **		   Remoteness, SetRemoteness
 **
 **	Logic:
 **			Go through all possible nodes finding the visited undecided positions
 **			See if you can find a value for these positions, if you cannot, it means
 **			one of two things. Either
 **			   a) the node you are at loops back up the tree to create a draw
 **			   b) more undecided nodes beneath current node need values
 **			In the event of (a) you can mark those nodes off as draws, however we
 **			dont know if (a) is true unless we can be sure that (b) is false
 **			We can say (b) is false when the number of undecided positions stops 
 **                     changing
 **
 **			Why is this true?
 **			    Suppose (b) holds for some node J. This means that there is some
 **			    node beneath node J that is undecided and can be assigned a value. We
 **			    can call this node W. Once node W is assigned a value, 
 **			    the total number of undecided nodes in the tree 
 **                         decreases(thus differs).
 **				
 **                     But what happens if you add the same number of undecided nodes to the tree
 **			as you find values for?
 **			    Simple, if you have added new nodes, iter again, eventually you will
 **			    have to stop adding new nodes, and the number of undecided positions
 **			    can only decrease(if they change at all)
 **
 **			Eventually, the number of undecided nodes will stop changing, either 
 **                     through solving all of the nodes, or the proof for (b).
 **			Then all remaining nodes that are visited and undecided are set to draws
 **************/

VALUE DetermineZeroValue(POSITION position)
{
    POSITION i,lowSeen,highSeen;
    POSITION numUndecided, oldNumUndecided, numNew;
    MOVELIST *moveptr, *headMove;
    POSITION child;
    VALUE childValue;
    POSITION numTot, numWin, numTie;
    int tieRemoteness, winRemoteness;
    
    if (gTwoBits)
        InitializeVisitedArray();
    
    StoreValueOfPosition(position,Primitive(position));
    MarkAsVisited(position);
    oldNumUndecided = 0;
    numUndecided = 1;
    numNew = 1;
    
    lowSeen = position;
    highSeen = lowSeen+1;

    while((numUndecided != oldNumUndecided) || (numNew != 0)){
        
	oldNumUndecided = numUndecided;
        numUndecided = 0;
        numNew = 0;
        for(i = lowSeen; i <= highSeen;i++){
            if(Visited(i)){
                if(GetValueOfPosition(i) == undecided) {
                    moveptr = headMove = GenerateMoves(i);
                    numTot = numWin = numTie = 0;
                    tieRemoteness = winRemoteness = REMOTENESS_MAX;
                    while(moveptr != NULL){
                        child = DoMove(i,moveptr->move);
                        numTot++;
                        if(Visited(child))
                            childValue = GetValueOfPosition(child);
                        else{
                            childValue = Primitive(child);
                            numNew++;
                            MarkAsVisited(child);
                            StoreValueOfPosition(child,childValue);
                            if(childValue != undecided){
                                SetRemoteness(child,0);
                            }
                            if(child < lowSeen) lowSeen = child;
                            if(child > highSeen) highSeen = child + 1;
                        }
			
                        if(childValue == lose){
                            StoreValueOfPosition(i,win);
                            if(Remoteness(i) > Remoteness(child)+1)
                                SetRemoteness(i,Remoteness(child)+1);
                        }
			
                        if(childValue == win){
                            numWin++;
                            if(Remoteness(child) < winRemoteness){
                                winRemoteness = Remoteness(child);
                            }
                        }
                        if(childValue == tie){
                            numTie++;
                            if(Remoteness(child) < tieRemoteness){
                                tieRemoteness = Remoteness(child);
                            }
                        }
			
                        moveptr = moveptr -> next;
                    }
                    FreeMoveList(headMove);
                    if((numTot != 0) && (numTot == numWin + numTie)){
                        if(numTie == 0){
                            StoreValueOfPosition(i,lose);
                            SetRemoteness(i, winRemoteness+1);
                        }else{
                            StoreValueOfPosition(i,tie);
                            SetRemoteness(i, tieRemoteness+1);
                        }
                    }
		    
                    if(GetValueOfPosition(i) == undecided)
                        numUndecided++;
                }
            }
        }
	
        printf("\nnumUndecided: %d, diff: %d, numNew: %d, lowSeen: %d, highSeen: %d",numUndecided,numUndecided - oldNumUndecided,numNew,lowSeen,highSeen);
	
    }
    
    for(i = 0; i < gNumberOfPositions;i++){
        if(Visited(i) && (GetValueOfPosition(i) == undecided)){
            StoreValueOfPosition(i, tie);
            SetRemoteness(i,REMOTENESS_MAX);
        }
        UnMarkAsVisited(i);
    }
    
    return GetValueOfPosition(position);
}

/*************
 **  End ZeroSolver
 **
 **
 *************/