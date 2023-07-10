import numpy as np


class BayesianNetwork:
    def __init__(self, n_labels=10, n_pixels=784, n_values=2) -> None:
        '''
        n_labels: number of labels, 10 for digit recognition
        n_pixels: number of pixels, 256 for 16x16 image
        n_values: number of values for each pixel, 0 for black, 1 for white
        '''
        self.n_labels = n_labels
        self.n_pixels = n_pixels
        self.n_values = n_values
        # prior probability
        self.labels_prior = np.zeros(n_labels)
        # conditional probability
        self.pixels_cond_label = np.zeros((n_pixels, n_values, n_labels))

    # fit the model with training data
    def fit(self, pixels, labels):
        '''
        pixels: (n_samples, n_pixels, )
        labels: (n_samples, )
        '''
        n_samples = len(labels)
        # TODO: calculate prior probability and conditional probability
        # prior probability
        self.labels_prior[np.unique(labels)] = np.bincount(labels) / n_samples
        # conditional probability
        for i in range(self.n_labels):
            self.pixels_cond_label[:, 1, i] = \
                np.sum(pixels[labels == i], axis=0) / np.sum(labels == i)
            self.pixels_cond_label[:, 0, i] = \
                1 - self.pixels_cond_label[:, 1, i]

    # predict the labels for new data
    def predict(self, pixels):
        '''
        pixels: (n_samples, n_pixels, )
        return labels: (n_samples, )
        '''
        n_samples = len(pixels)
        labels = np.zeros(n_samples)
        # TODO: predict for new data
        pixels_cond_label_one = self.pixels_cond_label[:, 1, :]
        pixels_cond_label_zero = self.pixels_cond_label[:, 0, :]
        for i in range(n_samples):
            sample = np.transpose([pixels[i]])
            prob_one = np.multiply(sample, pixels_cond_label_one)
            prob_zero = np.multiply(1 - sample, pixels_cond_label_zero)
            prob = np.prod(prob_one + prob_zero, axis=0) * self.labels_prior
            labels[i] = np.argmax(prob)

        return labels

    # calculate the score (accuracy) of the model
    def score(self, pixels, labels):
        '''
        pixels: (n_samples, n_pixels, )
        labels: (n_samples, )
        '''
        n_samples = len(labels)
        labels_pred = self.predict(pixels)
        return np.sum(labels_pred == labels) / n_samples


if __name__ == '__main__':
    # load data
    train_data = np.loadtxt('../data/train.csv', delimiter=',', dtype=np.uint8)
    test_data = np.loadtxt('../data/test.csv', delimiter=',', dtype=np.uint8)
    pixels_train, labels_train = train_data[:, :-1], train_data[:, -1]
    pixels_test, labels_test = test_data[:, :-1], test_data[:, -1]
    # build bayesian network
    bn = BayesianNetwork()
    bn.fit(pixels_train, labels_train)
    print('test score: %f' % bn.score(pixels_test, labels_test))
