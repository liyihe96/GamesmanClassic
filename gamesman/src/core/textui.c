/************************************************************************
**
** NAME:	textui.c
**
** DESCRIPTION:	Contains menus and other functions to get user input
**		and determine what to do given an input.
**
** AUTHOR:	GamesCrafters Research Group, UC Berkeley
**		Supervised by Dan Garcia <ddgarcia@cs.berkeley.edu>
**
** DATE:	2005-01-11
**
** LICENSE:	This file is part of GAMESMAN,
**		The Finite, Two-person Perfect-Information Game Generator
**		Released under the GPL:
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program, in COPYING; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
**************************************************************************/

#include "gamesman.h"
#include <time.h>

/*
** Local function prototypes
*/

static void	MenusBeforeEvaluation		(void);
static void	MenusEvaluated			(void);
static void	ParseMenuChoice			(char c);
static BOOLEAN	ParseConstantMenuChoice		(char c);
static void	ParseBeforeEvaluationMenuChoice	(char c);
static void	ParseEvaluatedMenuChoice	(char c);
static void	ParseHelpMenuChoice		(char c);
static void	SmarterComputerMenu		(void);
static void	AnalysisMenu			(void);


/*
** Code
*/

void HitAnyKeyToContinue()
{
    static BOOLEAN first = TRUE;
    
    printf("\n\t----- Hit <return> to continue -----");
    first ? (first = FALSE) : getchar(); /* to make lint happy */
    while(getchar() != '\n');
}

char GetMyChar()
{
    char inString[MAXINPUTLENGTH], ans;
    scanf("%s",inString);
    ans = inString[0];
    return(ans);
}



void Menus()
{
    printf(kOpeningCredits,kSolveVersion,kGameName);

    HitAnyKeyToContinue();

    do {
        printf("\n\t----- GAMESMAN version %s with %s module -----\n", 
            kSolveVersion, kGameName);

        if(gMenuMode == BeforeEvaluation)
            MenusBeforeEvaluation();
        else if(gMenuMode == Evaluated)
            MenusEvaluated();

        printf("\n\th)\t(H)elp.\n");
        printf("\n\n\tq)\t(Q)uit.\n");
        printf("\n\nSelect an option: ");

        ParseMenuChoice(GetMyChar());
    } while(TRUE);
}

void MenusBeforeEvaluation()
{
    gUnsolved = FALSE;
    
    printf("\n\ts)\t(S)TART THE GAME\n");
    printf("\tw)\tSTART THE GAME (W)ITHOUT SOLVING\n");

    printf("\n\tEvaluation Options:\n\n");
    printf("\to)\t(O)bjective toggle from %s to %s\n",
        gStandardGame ? "STANDARD" : "REVERSE ",
        gStandardGame ? "REVERSE " : "STANDARD");
    if(kDebugMenu)
        printf("\td)\t(D)ebug Module BEFORE Evaluation\n");
    if(kGameSpecificMenu)
        printf("\tg)\t(G)ame-specific options for %s\n",kGameName);
    printf("\t2)\tToggle (2)-bit solving (currently %s)\n", gTwoBits ? "ON" : "OFF");
    printf("\tp)\tToggle Global (P)osition solving (currently %s)\n", gGlobalPositionSolver ? "ON" : "OFF");
    printf("\tl)\tToggle (L)ow Mem solving (currently %s)\n", gZeroMemSolver ? "ON" : "OFF");
    printf("\tm)\tToggle Sy(M)metries (currently %s)\n", gSymmetries ? "ON" : "OFF");
}

void MenusEvaluated()
{
    VALUE gameValue = undecided;
    
    if(!gUnsolved)
        gameValue = GetValueOfPosition(gInitialPosition);
    
    printf("\n\tPlayer Name Options:\n\n");
    
    printf("\t1)\tChange the name of player 1 (currently %s)\n",gPlayerName[1]);
    printf("\t2)\tChange the name of player 2 (currently %s)\n",gPlayerName[0]);
    if(gOpponent == AgainstHuman)
        printf("\t3)\tSwap %s (plays FIRST) with %s (plays SECOND)\n", gPlayerName[1], gPlayerName[0]);
    
    if(!gUnsolved) {
	printf("\n\tGeneric Options:\n\n");
	
	printf("\t4)\tToggle from %sPREDICTIONS to %sPREDICTIONS\n",
	       gPrintPredictions ? "   " : "NO ",
	       !gPrintPredictions ? "   " : "NO ");
	printf("\t5)\tToggle from %sHINTS       to %sHINTS\n",
	       gHints ? "   " : "NO ",
	       !gHints ? "   " : "NO ");
    }
    
    printf("\n\tPlaying Options:\n\n");
    if(!gUnsolved) {
	printf("\t6)\tToggle opponents, currently: %s\n",
	       (gOpponent == AgainstComputer) ? "PLAYING A COMPUTER" : 
	       (gOpponent == AgainstHuman) ? "PLAYING A HUMAN" : 
	       "COMPUTER PLAYING A COMPUTER");
    }
    if(gOpponent == AgainstComputer)
	{
	    if(gameValue == tie)
		printf("\t7)\tToggle from going %s (can tie/lose) to %s (can tie/lose)\n",
		       gHumanGoesFirst ? "FIRST" : "SECOND",
		       gHumanGoesFirst ? "SECOND" : "FIRST");
	    else if (gameValue == lose)
		printf("\t7)\tToggle from going %s (can %s) to %s (can %s)\n",
		       gHumanGoesFirst ? "FIRST" : "SECOND",
		       gHumanGoesFirst ? "only lose" : "win/lose",
		       gHumanGoesFirst ? "SECOND" : "FIRST",
		       gHumanGoesFirst ? "win/lose" : "only lose");
	    else if (gameValue == win)
		printf("\t7)\tToggle from going %s (can %s) to %s (can %s)\n",
		       gHumanGoesFirst ? "FIRST" : "SECOND",
		       gHumanGoesFirst ? "win/lose" : "only lose",
		       gHumanGoesFirst ? "SECOND" : "FIRST",
		       gHumanGoesFirst ? "only lose" : "win/lose");
	    else
		BadElse("Menus");
	    
	    printf("\n\ta)\t(A)nalyze the game\n");
	    printf("\tc)\tAdjust (C)omputer's brain (currently ");
	    if (smartness==SMART) {
		printf("%d%% perfect", scalelvl);
	    }
	    else if (smartness==DUMB) {
		printf("misere-ly");
	    }
	    else if (smartness==RANDOM) {
	    printf("randomly");
	    }
	    printf (" w/%d giveback%s)\n", initialGivebacks, initialGivebacks == 1 ? "" : "s");
	}

    printf("\n\ts)\tIf only one move is available, (S)kip user input (currently %s)\n",
	   gSkipInputOnSingleMove? "ON" : "OFF");

    if(kDebugMenu)
        printf("\td)\t(D)ebug Game AFTER Evaluation\n");
    printf("\n\tp)\t(P)LAY GAME.\n");
    
    printf("\n\tm)\tGo to (M)ain Menu to edit game rules or starting position.\n");
}

void ParseMenuChoice(char c)
{
    if (ParseConstantMenuChoice(c));
    else if(gMenuMode == BeforeEvaluation)
        ParseBeforeEvaluationMenuChoice(c);
    else if(gMenuMode == Evaluated)
        ParseEvaluatedMenuChoice(c);
    else {
        BadElse("ParseMenuChoice");
        HitAnyKeyToContinue();
    }
}

BOOLEAN ParseConstantMenuChoice(char c)
{
    switch(c) {
  case 'Q': case 'q':
      ExitStageRight();
      exit(0);
  case 'h': case 'H':
      HelpMenus();
      break;
  default:
      return(FALSE);  /* It was not parsed here */
    }
    return(TRUE);       /* Yep, it was parsed here! */
}

void ParseBeforeEvaluationMenuChoice(char c)
{

    BOOLEAN tempPredictions;
    VALUE gameValue;
    char mexString[20]; /* Mex values read "[Val = *14]", easily in 20 chars */
    char tmpString[80]; /* Just for saving the remoteness value in it */

    switch(c) {
    case 'G': case 'g':
	if(kGameSpecificMenu)
      {
          tempPredictions = gPrintPredictions ;
          gPrintPredictions = FALSE ;
          GameSpecificMenu();
          gPrintPredictions = tempPredictions ;
      }
	else {
	    BadMenuChoice();
	    HitAnyKeyToContinue();
	}
	break;
    case '2':
	gTwoBits = !gTwoBits;
	break;
    case 'p': case 'P':
        gGlobalPositionSolver = !gGlobalPositionSolver;
        break;
    case 'l': case 'L':
	gZeroMemSolver = !gZeroMemSolver;
	break;
    case 'm': case 'M':
	gSymmetries = !gSymmetries;
	break;
    case 'o': case 'O':
	gStandardGame = !gStandardGame;
	break;
    case 'D': case 'd':
	if(kDebugMenu)
	    DebugModule();
	else
	    BadMenuChoice();
	break;
    case 's': case 'S':
	Initialize();
	gOpponent = AgainstComputer;
	gPrintPredictions = TRUE;
	sprintf(gPlayerName[kPlayerOneTurn],"Player");
	sprintf(gPlayerName[kPlayerTwoTurn],"Computer");
	printf("\nSolving with loopy code %s...%s!",kGameName,kLoopy?"Yes":"No");
	if (kLoopy && gGoAgain!=DefaultGoAgain) printf(" with Go Again support");
	printf("\nSolving with ZeroSolver %s...%s!",kGameName,gZeroMemSolver?"Yes":"No");
	printf("\nRandom(100) three times %s...%d %d %d",kGameName,GetRandomNumber(100),GetRandomNumber(100),GetRandomNumber(100));
	printf("\nInitializing insides of %s...", kGameName);
	fflush(stdout);
	Stopwatch();
	InitializeDatabases();
	printf("done in %u seconds!", Stopwatch()); // for analysis bookkeeping
	fflush(stdout);
	Stopwatch();
	gPrintDatabaseInfo = TRUE;
	gameValue = DetermineValue(gInitialPosition);
	printf("done in %u seconds!\e[K", gAnalysis.TimeToSolve = Stopwatch()); /* Extra Spacing to Clear Status Printing */
	if((Remoteness(gInitialPosition)) == REMOTENESS_MAX){
	    printf("\n\nThe Game %s has value: Draw\n\n", kGameName);
	} else {
	  MexFormat(gInitialPosition, mexString); /* Mex value not so well-defined for draws */
	  sprintf(tmpString, "in %d", Remoteness(gInitialPosition));
	  printf("\n\nThe Game %s has value: %s %s %s\n\n", 
		 kGameName, 
		 gValueString[(int)gameValue], 
		 gTwoBits ? "" : tmpString, /* TwoBit solvers have no remoteness */
		 mexString);
	}
	gMenuMode = Evaluated;
	if(gameValue == lose)
	    gHumanGoesFirst = FALSE;
	else
	    gHumanGoesFirst = TRUE ;
	HitAnyKeyToContinue();
	break;
    case 'w': case 'W':
	InitializeGame();
	gUnsolved = TRUE;
	gOpponent = AgainstHuman;
	gPrintPredictions = FALSE;
	sprintf(gPlayerName[kPlayerOneTurn],"Player 1");
	sprintf(gPlayerName[kPlayerTwoTurn],"Player 2");
	printf("\n\nYou have chosen to play the game without solving.  Have fun!\n\n");
	gMenuMode = Evaluated;
	HitAnyKeyToContinue();
	break;
    default:
	BadMenuChoice();
	HitAnyKeyToContinue();
	break;
    }
}

void ParseEvaluatedMenuChoice(char c)
{
    char tmpName[MAXNAME];
    PLAYER playerOne,playerTwo;
    
    switch(c) {
    case '1':
	printf("\nEnter the name of player 1 (max. %d chars) [%s] : ",
	       MAXNAME-1, gPlayerName[kPlayerOneTurn]);
	GetMyString(tmpName,MAXNAME,TRUE,FALSE);
	if(strcmp(tmpName,""))
	    (void) sprintf(gPlayerName[kPlayerOneTurn],"%s",tmpName);
	break;
    case '2':
	printf("\nEnter the name of player 2 (max. %d chars) [%s] : ",
	       MAXNAME-1, gPlayerName[kPlayerTwoTurn]);
	GetMyString(tmpName,MAXNAME,TRUE,FALSE);
	if(strcmp(tmpName,""))
	    (void) sprintf(gPlayerName[kPlayerTwoTurn],"%s",tmpName);
	break;
    case '3':
	(void) sprintf(tmpName,"%s",gPlayerName[kPlayerOneTurn]);
	(void) sprintf(gPlayerName[kPlayerOneTurn],"%s",gPlayerName[kPlayerTwoTurn]);
	(void) sprintf(gPlayerName[kPlayerTwoTurn],"%s",tmpName);
	break;
    case '4':
	if(gUnsolved) {
	    BadMenuChoice();
	    HitAnyKeyToContinue();
	}
	gPrintPredictions = !gPrintPredictions;
	break;
    case '5':
	if(gUnsolved) {
	    BadMenuChoice();
	    HitAnyKeyToContinue();
	}
	gHints = !gHints;
	break;
    case '6':
	if(gUnsolved) {
	    BadMenuChoice();
	    HitAnyKeyToContinue();
	}
	if(gOpponent == AgainstComputer) {
	    gOpponent = AgainstHuman;
	}
	else if(gOpponent == AgainstHuman) {
	    gOpponent = ComputerComputer;
	}
	else if(gOpponent == ComputerComputer) {
	    gOpponent = AgainstComputer;
	}
	else
	    BadElse("Invalid Opponent!");
	break;
    case '7':
	if(gOpponent == AgainstComputer)
	    gHumanGoesFirst = !gHumanGoesFirst;
	else {
	    BadMenuChoice();
	    HitAnyKeyToContinue();
      }
	break;
    case 'D': case 'd':
	if(kDebugMenu)
	    DebugMenu();
	else {
	    BadMenuChoice();
	    HitAnyKeyToContinue();
	}
      break;
    case 'C': case 'c':
	SmarterComputerMenu();
	break;
    case 'A': case 'a':
        //analyze();
	AnalysisMenu();
	break;
    case 'p': case 'P':
	if(gOpponent == AgainstComputer) {
	    if(gHumanGoesFirst) {
		playerOne = NewHumanPlayer(gPlayerName[kPlayerOneTurn],0);
		playerTwo = NewComputerPlayer(gPlayerName[kPlayerTwoTurn],1);
	    } else {
		playerOne = NewComputerPlayer(gPlayerName[kPlayerTwoTurn],0);
		playerTwo = NewHumanPlayer(gPlayerName[kPlayerOneTurn],1);
	    }	    
	    PlayGame(playerOne,playerTwo);
	}
	else if(gOpponent == AgainstHuman) {
	    playerOne = NewHumanPlayer(gPlayerName[kPlayerOneTurn],0);
	    playerTwo = NewHumanPlayer(gPlayerName[kPlayerTwoTurn],1);
	    PlayGame(playerOne,playerTwo);
	}
	else if(gOpponent == ComputerComputer) {
	    playerOne = NewComputerPlayer(gPlayerName[kPlayerOneTurn],0);
	    playerTwo = NewComputerPlayer(gPlayerName[kPlayerTwoTurn],1);
	    PlayGame(playerOne,playerTwo);
	}
	else
	    BadElse("Invalid Opponent to play!");
      HitAnyKeyToContinue();
      break;

    case 's': case 'S':
	gSkipInputOnSingleMove = !gSkipInputOnSingleMove;
	break;

    case 'm': case 'M':
	gMenuMode = BeforeEvaluation;
	break;
    default:
	BadMenuChoice();
	HitAnyKeyToContinue();
      break;
    }
}

void HelpMenus()
{
    char c;
    
    do {
        printf("\n\t----- HELP for %s module -----\n\n", kGameName);
	
        printf("\t%s Help:\n\n",kGameName);
	
        printf("\t1)\tWhat do I do on MY TURN?\n");
        printf("\t2)\tHow to tell the computer WHICH MOVE I want?\n");
        printf("\t3)\tWhat is the %s OBJECTIVE of %s?\n", 
            gStandardGame ? "STANDARD" : "REVERSE", kGameName);
        printf("\t4)\tIs a TIE possible?\n");
        printf("\t5)\tWhat does the VALUE of this game mean?\n");
        printf("\t6)\tShow SAMPLE %s game.\n",kGameName);
	
        printf("\n\tGAMESMAN Help:\n\n");
	
        printf("\t7)\tWhat is a game VALUE?\n");
        printf("\t8)\tWhat is EVALUATION?\n");
	
        printf("\n\tGeneric Options Help:\n\n");

        printf("\t9)\tWhat are PREDICTIONS?\n");
        printf("\t0)\tWhat are HINTS?\n");
	
        printf("\n\n\tb)\t(B)ack = Return to previous activity.\n");
        printf("\n\nSelect an option: ");
	
        ParseHelpMenuChoice(c = GetMyChar());
	
    } while(c != 'b' && c != 'B');
}

void ParseHelpMenuChoice(char c)
{
    switch(c) {
    case 'Q': case 'q':
	ExitStageRight();
	exit(0);
    case '1':
	printf("\n\t----- What do I do on MY TURN? -----\n\n");
	printf("%s\n",kHelpOnYourTurn);
	break;
  case '2':
      printf("\n\t----- How do I tell the computer WHICH MOVE I want? -----\n\n");
      printf("%s\n",kHelpTextInterface);
      break;
    case '3':
	printf("\n\t----- What is the %s OBJECTIVE of %s? -----\n\n", 
	       gStandardGame ? "STANDARD" : "REVERSE", kGameName);
	printf("%s\n",
	       gStandardGame ? kHelpStandardObjective : kHelpReverseObjective);
	break;
    case '4':
	printf("\n\t----- Is a TIE possible? -----\n\n");
	printf("%s\n",!kTieIsPossible ? "NO, a tie is not possible in this game." :
	       "YES, a tie is possible in this game. A tie occurs when");
	if(kTieIsPossible)
	    printf("%s\n",kHelpTieOccursWhen);
	break;
    case '5':
	printf("\n\t----- What does the VALUE of %s mean? -----\n\n",kGameName);
	if(gMenuMode == Evaluated) {
	    if(gOpponent == AgainstComputer)
		PrintComputerValueExplanation();
	    else
		PrintHumanValueExplanation();
	}
	else {
	    printf("%s\n", kHelpValueBeforeEvaluation);
	}
	break;
    case '6':
	printf("\n\t----- Show SAMPLE %s game {Your answers in curly brackets} -----\n\n",kGameName);
	printf("%s\n",kHelpExample);
	break;
    case '7':
	printf("\n\t----- What is a game VALUE? -----\n\n");
	printf("%s\n",kHelpWhatIsGameValue);
	break;
    case '8':
	printf("\n\t----- What is EVALUATION? -----\n\n");
	printf("%s\n",kHelpWhatIsEvaluation);
      break;
    case '9':
	printf("\n\t----- What are PREDICTIONS? -----\n\n");
	printf("%s\n",kHelpWhatArePredictions);
	break;
    case '0':
	printf("\n\t----- What are HINTS? -----\n\n");
	printf("%s\n",kHelpWhatAreHints);
      break;
    case 'b': case 'B':
	return;
    default:
	BadMenuChoice();
	break;
    }
    HitAnyKeyToContinue();
}

void BadMenuChoice()
{
    printf("\nSorry, I don't know that option. Try another.\n");
}

/* Jiong */
void SmarterComputerMenu()
{
    char c;
    do {
        printf("\n\t----- Adjust COMPUTER'S brain menu for %s -----\n\n", kGameName);
        printf("\tp)\t(P)erfectly always\n");
        printf("\ti)\t(I)mperfectly (Perfect some %% of time, Randomly others)\n");
        printf("\tr)\t(R)andomly always\n");
        printf("\tm)\t(M)isere-ly always (i.e., trying to lose!)\n\n");
        printf("\tg)\tChange the number of (G)ive-backs (currently %d)\n\n", initialGivebacks);
        printf("\th)\t(H)elp\n\n");
        printf("\tb)\t(B)ack = Return to previous activity\n\n");
        printf("\tq)\t(Q)uit.\n");
        printf("\nSelect an option: ");
	
        switch(c = GetMyChar()) {
	case 'Q': case 'q':
	    ExitStageRight();
	    exit(0);
	case 'P': case 'p':
	    smartness = SMART;
	    scalelvl = 100;
	    HitAnyKeyToContinue();
	    break;
	case 'I': case 'i':
	    smartness = SMART;
	    printf("\nPlease enter the chance %% of time the computer plays perfectly (0-100): ");
	    scanf("%d", &scalelvl);
	    while (scalelvl < 0 || scalelvl > 100) {
		printf("\nPlease enter the chance %% of time the computer plays perfectly (0-100): ");
		scanf("%d", &scalelvl);
	    }
	    HitAnyKeyToContinue();
	    break;
	case 'R': case 'r':
	    smartness = RANDOM;
	    HitAnyKeyToContinue();
	    break;
	case 'M': case 'm':
	    smartness = DUMB;
	    HitAnyKeyToContinue();
	    break;
	case 'G': case 'g':
	    printf("\nPlease enter the number of give-backs the computer will perform (0-%d): ", MAXGIVEBACKS);
	    scanf("%d", &initialGivebacks);
	    while (initialGivebacks > MAXGIVEBACKS || initialGivebacks < 0) {
		printf("\nPlease enter the number of give-backs the computer will perform (0-%d): ", MAXGIVEBACKS);
		scanf("%d", &initialGivebacks);
	    } 
	    remainingGivebacks = initialGivebacks;
	    HitAnyKeyToContinue();
	    break;
	case 'H': case 'h':
	    HelpMenus();
	    break;
	case 'B': case 'b':
	    return;
	default:
	    BadMenuChoice();
	    HitAnyKeyToContinue();
	    break;
        }
    } while(TRUE);
}

void AnalysisMenu()
{
    POSITIONLIST *badWinPositions = NULL, *badTiePositions = NULL, *badLosePositions = NULL;
    BOOLEAN tempPredictions = gPrintPredictions, CorruptedValuesP();
    MEX mexValue = 0;
    int mexInt, maxPositions = 10;
    char c;
    
    gPrintPredictions = FALSE;
    gMenuMode = Analysis; /* By default, no symmetries -- raw values */
    
    do {
        printf("\n\t----- Post-Evaluation ANALYSIS menu for %s -----\n\n", kGameName);
        printf("\ti)\tPrint the (I)nitial position\n");
        printf("\tn)\tChange the (N)umber of printed positions (currently %d)\n",maxPositions);
        if(!kPartizan) { /* Impartial */
            printf("\tv)\tChange the Mex (V)alue (currently %d)\n",(int)mexValue);
            printf("\n\tm)\tPrint up to %d positions with (M)ex value %d\n",maxPositions,(int)mexValue);
        } else
            printf("\n");
        printf("\tw)\tPrint up to %d (W)inning positions\n",maxPositions);
        printf("\tl)\tPrint up to %d (L)osing  positions\n",maxPositions);
        printf("\tt)\tPrint up to %d (T)ieing  positions\n",maxPositions);
        printf("\n\tp)\t(P)rint the overall summmary of game values\n");
        printf("\tf)\tPrint to an ascii (F)ile the raw game values + remoteness\n");
        printf("\to)\tPrint to std(O)ut the raw game values + remoteness\n");
	
        printf("\td)\tPrint (D)atabase comb visualization (POSitions vs NEGativeSpace)\n");
        printf("\n\tc)\t(C)heck if value database is corrupted\n");
	
        if(badWinPositions != NULL)
            printf("\t1)\tPrint up to %d (W)inning INCORRECT positions\n",maxPositions);
        if(badTiePositions != NULL)
            printf("\t2)\tPrint up to %d (T)ieing  INCORRECT positions\n",maxPositions);
        if(badLosePositions != NULL)
            printf("\t3)\tPrint up to %d (L)osing  INCORRECT positions\n",maxPositions);
	
        printf("\n\th)\t(H)elp\n");
        printf("\n\tb)\t(B)ack = Return to previous activity\n");
        printf("\n\nSelect an option: ");
	
        switch(c = GetMyChar()) {
	case 'Q': case 'q':
	    ExitStageRight();
	    exit(0);
	case 'H': case 'h':
	    HelpMenus();
	    break;
	case 'D': case 'd':
	    DatabaseCombVisualization();
	    break;
	case 'C': case 'c':
	    gMenuMode = Evaluated;
	    if(CorruptedValuesP())
		printf("\nCorrupted values found and printed above. Sorry.\n");
	    else
		printf("\nNo Corrupted Values found!\n");
	    gMenuMode = Analysis;
	    HitAnyKeyToContinue();
	    break;
	case 'F': case 'f':
	    PrintRawGameValues(TRUE);
	    break;
	case 'O': case 'o':
	    PrintRawGameValues(FALSE);
	    HitAnyKeyToContinue();
	    break;
	case 'i': case 'I':
	    printf("\n\t----- The Initial Position is shown below -----\n");
	    PrintPosition(gInitialPosition, "Nobody", TRUE);
	    HitAnyKeyToContinue();
	    break;
	case 'p': case 'P':
	    analyze();
	    PrintGameValueSummary();
	    HitAnyKeyToContinue();
	    break;
	case 'm': case 'M':
	    if(!kPartizan) /* Impartial */
		PrintMexValues(mexValue,maxPositions);
	    else {
		BadMenuChoice();
		HitAnyKeyToContinue();
	    }
	    break;
	case 'n': case 'N':
	    printf("\nPlease enter the MAX number of positions : ");
	    scanf("%d", &maxPositions);
	    break;
	case 'v': case 'V':
	    if(!kPartizan) { /* Impartial */
		printf("\nPlease enter the MEX number : ");
		scanf("%d", &mexInt);
		mexValue = (MEX) mexInt;
	    } else {
		BadMenuChoice();
		HitAnyKeyToContinue();
	    }
	    break;
	case 'w': case 'W': case 'l': case 'L': case 't': case 'T':
	    PrintValuePositions(c,maxPositions);
	    break;
	case '1': case '2': case '3':
	    PrintBadPositions(c,maxPositions,badWinPositions, badTiePositions, badLosePositions);
	    break;
	case 'b': case 'B':
	    FreePositionList(badWinPositions);
	    FreePositionList(badTiePositions);
	    FreePositionList(badLosePositions);
	    gPrintPredictions = tempPredictions;
	    gMenuMode = Evaluated; /* Return to simple 'Evaluated' mode/menu */
	    return;
	default:
	    BadMenuChoice();
	    HitAnyKeyToContinue();
	    break;
        }
    } while(TRUE);
}

USERINPUT HandleDefaultTextInput(POSITION thePosition, MOVE* theMove, STRING playerName)
{
    MOVE tmpMove;
    char tmpAns[2], input[MAXINPUTLENGTH];
    MOVELIST* head;
    int onlyOneMove;
    
    /*to skip input we have to see what moves are available first, and since we
      are not solving there is esstially no performance issue here*/
    head = GenerateMoves(thePosition); /* What are all moves available? */
    onlyOneMove = (head != NULL && head->next == NULL);
    *theMove = head->move;
    if ( gSkipInputOnSingleMove && onlyOneMove ) {
        printf("\n---------- SELECTING THE ONLY MOVE ---------> ");
        PrintMove(*theMove);
        printf("\n");
	return (Move);
    }
    FreeMoveList(head);

    GetMyString(input,MAXINPUTLENGTH,TRUE,TRUE);
    
    if(input[0] == '\0') {
      /* [DDG 2005-01-09] Check if there is only one move to be made.
       * If so, this can be a shortcut for moving, just hitting enter! */
      /*      head = GenerateMoves(thePosition); What are all moves available? */
      /* There's exactly one */
      /*if (onlyOneMove = (head != NULL && head->next == NULL)){
       *  *theMove = head->move;
       * printf("----- AUTO-MOVE-SELECTED ------------> ");
       * PrintMove(*theMove);
       * printf("\n");
       *}
       *FreeMoveList(head);*/
	
	if ( onlyOneMove ) {
	    printf("\n-------- SELECTING THE ONLY MOVE --------> ");
	    PrintMove(*theMove);
	    printf("\n");
	    return(Move);
	} else
	    PrintPossibleMoves(thePosition);
	
    } else if (ValidTextInput(input)) {
        if(ValidMove(thePosition,tmpMove = ConvertTextInputToMove(input))) {
            *theMove = tmpMove;
            return(Move);
        }
        else
            PrintPossibleMoves(thePosition);
    
    } else {
        switch(input[0]) {
	case 'Q': case 'q':
	    ExitStageRight();
	    exit(0);
	case 'u': case 'U':
	    return(Undo);
	case 'a': case 'A':
	    printf("\nSure you want to Abort? [no] :  ");
	    GetMyString(tmpAns,2,TRUE,TRUE);
	    printf("\n");
	    if(tmpAns[0] == 'y' || tmpAns[0] == 'Y')
		return(Abort);
	    else
		return(Continue);
	case 'H': case 'h':
	    HelpMenus();
	    printf("");
	    PrintPosition(thePosition, playerName, TRUE);
	    break;
	case 'c': case 'C':
	    SmarterComputerMenu();
	    break;
	case 'r': case 'R':
	    PrintPosition(thePosition, playerName, TRUE);
	    break;
	case 's': case 'S':
	    if (gUnsolved) {
		printf("Sorry, we cannot give you these values before the game is solved.\n");
		break;
	    } else {
		PrintValueMoves(thePosition);
		break;
	    }
	case 'p': case 'P':
	    if (gUnsolved) {
		printf("Sorry, we cannot make predictions before the game is solved.\n");
		break;
	    } else {
		gPrintPredictions = !gPrintPredictions;
		printf("\n Predictions %s\n", gPrintPredictions ? "On." : "Off.");
		PrintPosition(thePosition, playerName, TRUE);
		break;
	    }
	case 'v': case 'V':
	  PrintVisualValueHistory(thePosition);
	  break;
	case 't': case 'T':
	  PrintMoveHistory(thePosition);
	  break;
	case '?':
	    printf("%s",kHandleDefaultTextInputHelp);
	    PrintPossibleMoves(thePosition);
	    break;
	default:
	    BadMenuChoice();
	    printf("%s",kHandleDefaultTextInputHelp);
	    PrintPossibleMoves(thePosition);
	    break;
        }
    }
    
    return(Continue);  /* The default action is to return Continue */
}


void GetMyString(char* name, int size, BOOLEAN eatFirstChar, BOOLEAN putCarraigeReturnBack)
{	
    int ctr = 0;
    BOOLEAN seenFirstNonSpace = FALSE;
    signed char c;
    
    if(eatFirstChar)
        (void) getchar();
    
    while((c = getchar()) != '\n' && c != EOF) {
	
        if(!seenFirstNonSpace && c != ' ')
            seenFirstNonSpace = TRUE;
	
        if(ctr < size - 1 && seenFirstNonSpace)
            name[ctr++] = c;
    }
    name[ctr] = '\0';
    
    if(putCarraigeReturnBack)
        ungetc('\n',stdin);  /* Put the \n back on the input */
}

/* Status Meter */
void showStatus(STATICMESSAGE msg)
{
    
    static float timeDelayTicks = CLOCKS_PER_SEC / 10;
    static clock_t updateTime = (clock_t) NULL;
    int print_length=0;
    float percent = PercentDone(msg);
    
    if (updateTime == (clock_t) NULL)
    {
        updateTime = clock() + timeDelayTicks; /* Set Time for the First Time */
    }
    
    switch (msg)
    {
        case Clean:
            if(gSaveDatabase) 
            {
                print_length = fprintf(stderr,"Writing Database...\e[K");
                fprintf(stderr,"\e[%dD",print_length - 3); /* 3 Characters for the escape sequence */
            }
            updateTime = (clock_t) NULL;
            return;
       default:
         break;
    }	
    
    if (clock() > updateTime)
    {
        fflush(stdout);
        fflush(stderr);
        print_length = fprintf(stderr,"%2.1f%% Done \e[K",percent);
        fprintf(stderr,"\e[%dD",print_length - 3); /* 3 Characters for the escape sequence */
        updateTime = clock() + timeDelayTicks; /* Get the Next Update Time */
    }
}
