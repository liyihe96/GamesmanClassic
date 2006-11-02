package patterns;

import game.Bin;
import game.Move;
import game.Position;

public class Replace extends SimplePattern
{
    public static boolean isMatch(int oldPiece, int newPiece)
    {
	//return !oldPiece.isEmpty() && !newPiece.isEmpty() && !oldPiece.equals( newPiece );
	/*	if(oldPiece!=Bin.EMPTY && newPiece!=Bin.EMPTY && oldPiece!=newPiece)
	    return true;
	else
	return false;*/

	return (oldPiece!=Bin.ID_EMPTY && newPiece!=Bin.ID_EMPTY && oldPiece!=newPiece);
    }

    protected boolean matches( Move m ) {
	String firstPos = m.GetPositionBeforeMove().toString();
	String secondPos = m.GetPositionAfterMove().toString();

	int count = 0;
	int size = firstPos.length();

	Bin firstPlace;
	Bin secondPlace;

	for( int i = 0; i < size; ++i ) {
	    firstPlace = Bin.getArchetype( firstPos.charAt( i ) );
	    secondPlace = Bin.getArchetype( secondPos.charAt( i ) );

	    if( firstPlace.getID() == secondPlace.getID() )
		continue;
	    else if( !firstPlace.isEmpty() && !secondPlace.isEmpty() ) {
		// we know that the positions are different and the both are unempty
		// This was a replace.
		from = to = m.GetPositionBeforeMove().getLocationCoords( i );

		++count;
		break;

	    } else {
		// beginning or end is empty, not a replace.
	    }
	}

	if( count == 1 )
	    return true;
	else
	    return false;
    }

    public String toString() {
	return "replace";
    }

}
