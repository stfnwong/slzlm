# lets do a simple brute-force KNN to see what happens

import numpy as np
from scipy import stats


from sklearn.preprocessing import StandardScaler
from sklearn.model_selection import train_test_split
from sklearn.datasets import load_iris
from sklearn.neighbors import KNeighborsClassifier
from sklearn.metrics import accuracy_score, confusion_matrix, classification_report

import matplotlib.pyplot as plt
import seaborn as sns


class BruteKNN:
    def __init__(self, k: int, problem: int=0, metric: int=0):
        """
        k: number of nearest neighbours
        problem: 0 = regression, 1 = classification
        metric: 0 = Euclidean, 1 = Manhattan
        """

        self.k = k
        self.problem = problem
        self.metric = metric

    def fit(self, X_train: np.ndarray, y_train: np.ndarray):
        self.X_train = X_train
        self.y_train = y_train

    def predict(self, X_test: np.ndarray):

        m = self.X_train.shape[0]
        n = X_test.shape[0]

        y_pred = []

        # Compute distances between every sample in X_train and X_test
        for i in range(n):
            distance = []
            for j in range(m):
                if self.metric == 0:
                    d = np.sqrt(np.sum(np.square(X_test[i,:] - self.X_train[j,:])))
                else:
                    d = np.abs(X_test[i, :] - self.X_train[j,:])

                distance.append((d, self.y_train[j]))

            # Put distances in sorted order
            distance.sort()

            # Find k nearest neighbours
            nbors = []
            for n in range(self.k):
                nbors.append(distance[n][1])    # append the labels in the training set

            # Now predict
            if self.problem == 0:
                y_pred.append(np.mean(nbors))   # Regression
            else:
                y_pred.append(stats.mode(nbors)[0][0])   # Classification

        return y_pred




# Trial this against scipy


def main():
    iris = load_iris()
    data = iris.data
    target = iris.target


    # Do a train test split
    X_train, X_test, y_train, y_test = train_test_split(data, target, test_size=0.2)

    scaler = StandardScaler()

    X_train_scaled = scaler.fit_transform(X_train)
    X_test_scaled = scaler.fit_transform(X_test)

    # Find an optimal value for k.
    # We do this by instantiating a base KNN classifier with default settings and
    # making predictions for values of K in some range. Here we range from 1 to 31.

    train_error = []
    test_error = []

    min_k = 1
    max_k = 31

    for k in range(min_k, max_k+1):
        knn = KNeighborsClassifier(n_neighbors=k)
        knn.fit(X_train_scaled, y_train)

        # Get train and test error
        yp_train = knn.predict(X_train_scaled)
        yp_test = knn.predict(X_test_scaled)

        train_error.append(np.mean(y_train != yp_train))
        test_error.append(np.mean(y_test != yp_test))

    #from pudb import set_trace; set_trace()
    # Plot cuves

    plt.figure(figsize=(10, 5))
    plt.plot(train_error, color='b', label='train')
    plt.plot(test_error, color='r', label='test')

    #plt.plot(range(1, 15), train_error, color='b', label='train')
    #plt.plot(range(1, 15), test_error, color='r', label='test')

    plt.xlabel("k", fontsize=14)
    plt.ylabel("Error", fontsize=14)
    plt.title("Error as k increases", fontsize=18, pad=15)
    plt.legend()
    plt.show()


    # Take k that minimizes test error

    best_k = test_error.index(min(test_error)) + 1    # offset since index 0 is k=1
    best_knn = KNeighborsClassifier(n_neighbors=best_k)

    best_knn.fit(X_train_scaled, y_train)
    y_pred = best_knn.predict(X_test_scaled)
    acc = np.round(accuracy_score(y_test, y_pred), 5)


    print(f"Best k: {best_k}, Accuracy: {acc}")

    # Plot confusion matrix 
    conf_matrix = confusion_matrix(y_test, y_pred)
    fig, ax = plt.subplots()
    sns.heatmap(conf_matrix, annot=True, linewidth=3, cmap="viridis")
    ax.xaxis.tick_top()
    plt.xlabel("Predicted", fontsize=14, rotation=0, labelpad=30)
    plt.ylabel("True", fontsize=14, rotation=0, labelpad=30)
    plt.yticks(rotation=0)

    plt.title("KNN on iris confusion matrix", fontsize=14, pad=10)

    plt.show()

    print(classification_report(y_test, y_pred))


if __name__ == "__main__":
    main()
