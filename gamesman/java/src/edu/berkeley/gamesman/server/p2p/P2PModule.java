package edu.berkeley.gamesman.server.p2p;

import edu.berkeley.gamesman.server.IModule;
import edu.berkeley.gamesman.server.IModuleRequest;
import edu.berkeley.gamesman.server.IModuleResponse;
import edu.berkeley.gamesman.server.ModuleException;

public class P2PModule implements IModule
{

	public P2PModule()
	{
		super();
		// TODO Auto-generated constructor stub
	}

	public boolean typeSupported(String requestTypeName)
	{
		// TODO Auto-generated method stub
		return false;
	}

	public void handleRequest(IModuleRequest req, IModuleResponse res) throws ModuleException
	{
		// TODO Auto-generated method stub

	}

}
