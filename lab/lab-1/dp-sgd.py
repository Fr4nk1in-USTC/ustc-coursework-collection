import numpy as np
from sklearn.datasets import load_breast_cancer, make_classification
from sklearn.metrics import accuracy_score
from sklearn.model_selection import train_test_split

RANDOM_STATE = 1


class LogisticRegressionCustom:
    def __init__(self, learning_rate=0.01, num_iterations=100):
        self.learning_rate = learning_rate
        self.num_iterations = num_iterations
        self.tau = 1e-6  # small value to prevent log(0)
        self.weights = None
        self.bias = None

    def sigmoid(self, z):
        # np.clip(z, -500, 500) limits the range of z to avoid extremely
        # large or small values that could lead to overflow.
        return 1 / (1 + np.exp(-np.clip(z, -700, 700)))

    def fit(self, X, y):
        # Initialize weights and bias
        num_samples, num_features = X.shape
        self.weights = np.zeros(num_features)
        self.bias = 0.0

        # Gradient descent optimization
        for _ in range(self.num_iterations):
            # Compute predictions of the model
            linear_model = np.dot(X, self.weights) + self.bias
            predictions = self.sigmoid(linear_model)

            # Compute loss and gradients
            loss = -np.mean(
                y * np.log(predictions + self.tau)
                + (1 - y) * np.log(1 - predictions + self.tau)
            )
            d_loss = -(
                y / (predictions + self.tau) - (1 - y) / (1 - predictions + self.tau)
            )
            dz = d_loss * (predictions * (1 - predictions))
            dw = np.dot(X.T, dz) / num_samples
            db = np.sum(dz) / num_samples

            # Update weights and bias
            self.weights -= self.learning_rate * dw
            self.bias -= self.learning_rate * db

    def dp_fit(self, X, y, epsilon, delta, C=1):
        # Initialize weights and bias
        num_samples, num_features = X.shape
        self.weights = np.zeros(num_features)
        self.bias = 0

        # TODO: Calculate epsilon_u, delta_u based epsilon, delta and epochs here.
        epsilon_u, delta_u = (epsilon / np.sqrt(self.num_iterations), delta)

        # Gradient descent optimization
        for _ in range(self.num_iterations):
            # Compute predictions of the model
            linear_model = np.dot(X, self.weights) + self.bias
            predictions = self.sigmoid(linear_model)

            # Compute loss and gradients
            loss = -np.mean(
                y * np.log(predictions + self.tau)
                + (1 - y) * np.log(1 - predictions + self.tau)
            )
            losses.append(loss)
            d_loss = -(
                y / (predictions + self.tau) - (1 - y) / (1 - predictions + self.tau)
            )
            dz = d_loss * (predictions * (1 - predictions))

            # TODO: Clip gradient here.
            clip_dz = clip_gradients(dz, C)
            # Add noise to gradients
            noisy_dz = add_gaussian_noise_to_gradients(clip_dz, epsilon_u, delta_u, C)

            dw = np.dot(X.T, noisy_dz) / num_samples
            db = np.sum(noisy_dz) / num_samples

            # Update weights and bias
            self.weights -= self.learning_rate * dw
            self.bias -= self.learning_rate * db

    def predict_probability(self, X):
        linear_model = np.dot(X, self.weights) + self.bias
        probabilities = self.sigmoid(linear_model)
        return probabilities

    def predict(self, X):
        probabilities = self.predict_probability(X)
        # Convert probabilities to classes
        return np.round(probabilities)


def get_train_data(dataset_name=None):
    if dataset_name is None:
        # Generate simulated data
        X, y = make_classification(
            n_samples=1000, n_features=20, n_classes=2, random_state=RANDOM_STATE
        )
    elif dataset_name == "cancer":
        # Load the breast cancer dataset
        data = load_breast_cancer()
        X, y = data.data, data.target
    else:
        raise ValueError("Not supported dataset_name.")

    # Normalize the data
    X = (X - X.mean(axis=0)) / X.std(axis=0)
    # Split the dataset into training and testing sets
    X_train, X_test, y_train, y_test = train_test_split(
        X, y, test_size=0.2, random_state=RANDOM_STATE
    )
    return X_train, X_test, y_train, y_test


def clip_gradients(gradients, C):
    # TODO: Clip gradients.
    if gradients.ndim == 1:
        clip_gradients = np.minimum(gradients, C)
    else:
        gradients_norm = np.linalg.norm(gradients, ord=2, axis=1)
        clip_base = np.maximum(gradients_norm / C, 1)
        clip_gradients = gradients / clip_base[:, np.newaxis]
    return clip_gradients


def add_gaussian_noise_to_gradients(gradients, epsilon, delta, C):
    # TODO: add gaussian noise to gradients.
    num_samples = gradients.shape[0]
    sigma = C * np.sqrt(2 * np.log(1.25 / delta)) / epsilon
    if gradients.ndim == 1:
        noisy_gradients = gradients + np.random.normal(0, sigma, gradients.shape)
    else:
        sum_gradients = np.sum(gradients, axis=0)
        noise = np.random.normal(0, sigma, sum_gradients.shape)
        noisy_gradients = (sum_gradients + noise) / num_samples
    return noisy_gradients


if __name__ == "__main__":
    np.random.seed(RANDOM_STATE)
    # Prepare datasets.
    dataset_name = "cancer"
    X_train, X_test, y_train, y_test = get_train_data(dataset_name)

    # Training the normal model
    normal_model = LogisticRegressionCustom(learning_rate=0.01, num_iterations=1000)
    normal_model.fit(X_train, y_train)
    y_pred = normal_model.predict(X_test)
    accuracy = accuracy_score(y_test, y_pred)
    print("Normal accuracy:", accuracy)

    # Training the differentially private model
    dp_model = LogisticRegressionCustom(learning_rate=0.01, num_iterations=1000)
    epsilon, delta = 0.5, 1e-3
    dp_model.dp_fit(X_train, y_train, epsilon=epsilon, delta=delta, C=1)
    y_pred = dp_model.predict(X_test)
    accuracy = accuracy_score(y_test, y_pred)
    print("DP accuracy:", accuracy)
