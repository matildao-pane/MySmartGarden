package projectiot;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.Map;
import java.util.TreeMap;

import org.eclipse.californium.core.CoapClient;
import org.eclipse.californium.core.CoapResponse;
import org.eclipse.californium.core.coap.MediaTypeRegistry;

public class Interface {
	
	static public boolean observeMode=false;
	static MyServer server = new MyServer(5683);
	static public Map<String,Resource> registeredResources = new TreeMap<String,Resource>();
	static public Map<String,ObservingCoapClient> observedResources = new TreeMap<String,ObservingCoapClient>();
	public static void main(String[] args) {
		
		startServer();
		
		BufferedReader input = new BufferedReader(new InputStreamReader(System.in));
		
		showMenu();
		
			
		while(true) {
			String command = "";
			try {
				
				command = input.readLine();
			
				if(!validCommand(command,6)) {
					System.out.println("invalid command");
					showMenu();
					continue;
				}
				
				if(registeredResources.size()==0) {
					System.out.println("No registered resources yet");
					continue;
				}
			
				int cmd = Integer.parseInt(command);
				switch(cmd) {
					case 0:
						showAvailableResources();
						break;
				
					case 1:
						changeGateStatus("ON","gate-actuator");
						break;
						
					case 2:
						changeGateStatus("OFF","gate-actuator");
						break;
						
					case 3:
						setWateringMode("irrigator-actuator");
						break;
						
					case 4:
						enterObservingState();
						break;
						
					case 5:
						System.exit(0);
						break;
						
					default:
						showMenu();
						break;
				}
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	}
	
		
	private static void startServer() {
		new Thread() {
			public void run() {
				
				server.start();
			}
		}.start();
	}
	
	private static boolean validCommand(String command, int i) {
		int p = i;
		if(command == null) {
			return false;
		}
		try {
			@SuppressWarnings("unused")
			Integer number = Integer.parseInt(command);
			if(number>p) {
				return false; 
			}
		}catch (NumberFormatException nfe) {
			return false;
		}
	return true;
	}

	
	public static void showMenu() {
		System.out.print("\n Please, insert a command: \n"
				+ "0 - show available resources and their status.\n"
				+ "1 - open the gate.\n"
				+ "2 - close the gate.\n"
				+ "3 - set watering mode.\n"
				+ "4 - enter observing mode.\n"
				+ "5 - exit.\n"
				+ "\n"
				+ ">>");
	}

	
	private static void enterObservingState() {
		System.out.println("|| You will now receive real time updates from the observable resources ||");
		System.out.println("|| Press q to go back ||\n");
		observeMode= true;
		BufferedReader input = new BufferedReader(new InputStreamReader(System.in));
		String command;
		while(true) {
			try {
				command = input.readLine();
				if(command.equals("q")) {
					observeMode = false;
					break;
				}
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
		showMenu();
	}

	private static void showAvailableResources() {
		for(Map.Entry<String,Resource> entry : registeredResources.entrySet()) {
			System.out.println(entry.getValue().toString());
		}
		showMenu();
	}


	private static void setWateringMode(String name) {
		BufferedReader input = new BufferedReader(new InputStreamReader(System.in));
		String auto = registeredResources.get(name).isAuto()?"AUTO":"MAN";
	
		System.out.print("\nchoose one mode: AUTO|MAN, actual value:"+auto+"\n"
			+ "1 - AUTOMATIC.\n"
			+ "2 - MANUAL.\n"
			);
		System.out.print("0 - go back\n");
		
		
		while(true) {
			String command = "";
			try {
				command = input.readLine();
			
				if(!validCommand(command,2)) {
					System.out.println("invalid command");
					showMenu();
					return;
				}
				int cmd = Integer.parseInt(command);
				switch(cmd) {
					case 0:
						showMenu();
						return;
					case 1:
						automode(name);
						break;
						
					case 2:
						manmode(name);
						break;
				
					default:
						return;
				}				
			} catch (IOException e) {
				e.printStackTrace();
			}

			auto = registeredResources.get(name).isAuto()?"AUTO":"MAN";
			System.out.print("choose one mode: AUTO|MAN, actual value:"+auto+"\n"
					+ "1 - AUTOMATIC.\n"
					+ "2 - MANUAL.\n"
					);
			System.out.print("0 - go back\n");
		}
	
	}
		
	private static void automode(String name) {
		
		if(!registeredResources.get(name).isAuto()) {
			System.out.println("irrigator in automatic mode\n");
			registeredResources.get(name).setAuto(true);
			changeIrrigatorStatus(name,"-", "AUTO");
		}else {
			System.out.println("already in automatic mode\n");
		}
	}

	private static void manmode(String name) {
		BufferedReader input = new BufferedReader(new InputStreamReader(System.in));
		
		String state = (registeredResources.get(name).getState())?"OFF":"ON";
		boolean st = (registeredResources.get(name).getState())?false:true;
		System.out.println("You are in manual mode.");
		System.out.println("1 - switch "+state+" the irrigator");
		System.out.print("0 - go back\n");
		while(true) {
			String command = "";
			try {
				command = input.readLine();
			
				if(!validCommand(command,1)) {
					System.out.println("invalid command");
					showMenu();
					return;
				}
				int cmd = Integer.parseInt(command);
				switch(cmd) {
					case 0:
						return;
						
					case 1:
						changeIrrigatorStatus(name, state, "MAN");
						System.out.println("Irrigator in manual mode switched "+state+".\n");
						registeredResources.get(name).setAuto(false);
						registeredResources.get(name).setState(st);
						return;
				
					default:
						break;
				}
				
			} catch (IOException e) {
				e.printStackTrace();
			}
			System.out.println("press 0 to go back , press 1 to switch"+state+" the irrigator");
		}		
	}
	
	private static void changeIrrigatorStatus(String name,String state, String auto) {
		
		CoapClient client = new CoapClient(registeredResources.get(name).getCoapURI());
		CoapResponse response = client.post("status="+state+"&auto="+auto, MediaTypeRegistry.TEXT_PLAIN);	
		
		//CoapResponse response = client.post("status="+state+",auto="+auto, MediaTypeRegistry.TEXT_PLAIN);	
		String code = response.getCode().toString();
		System.out.println(response.getResponseText());
		if(!code.startsWith("2")) {
			System.out.println("Error:"+code);
			return;
		}
	}
	
	private static void changeGateStatus(String mode, String name) {
		
		CoapClient client = new CoapClient(registeredResources.get(name).getCoapURI());

		CoapResponse response = client.post("status="+mode, MediaTypeRegistry.TEXT_PLAIN);
		String code = response.getCode().toString();

		if(!code.startsWith("2")) {
			System.out.println("Error:"+code);
			return;
		}
		if(mode.equals("ON")) {
			System.out.println("|| OPENING GATE ||\n");
			registeredResources.get(name).setState(true);
			
		}else {
			System.out.println("|| CLOSING GATE ||\n");
			registeredResources.get(name).setState(false);
	
		}
		showMenu();
	}
}