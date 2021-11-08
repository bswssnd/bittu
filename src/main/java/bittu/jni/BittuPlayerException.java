package bittu.jni;

public class BittuPlayerException extends Exception {
	private static final long serialVersionUID = 1L;

	protected int state;

	protected BittuPlayerException(int state, String message) {
		super(message);

		this.state = state;
	}

	public int getState() {
		return this.state;
	}

	public static void throwIfError(long ptr) throws BittuPlayerException {
		int state = BittuplayerJNI.getErrorState(ptr);
		var message = BittuplayerJNI.getErrorMessage(ptr);

		if (state != BittuplayerJNI.NO_ERROR) {
			throw new BittuPlayerException(state, message);
		}
	}
}
